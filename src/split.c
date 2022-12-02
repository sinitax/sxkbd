#include "hardware/regs/io_bank0.h"
#include "hardware/structs/padsbank0.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

#include "split.h"
#include "util.h"
#include "matrix.h"

#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/address_mapped.h"
#include "hardware/regs/intctrl.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/time.h"
#include "bsp/board.h"
#include "class/cdc/cdc_device.h"
#include "tusb.h"

#include <stdint.h>

#define UART_TIMEOUT 20
#define UART_BAUD 115200
#define UART_PIN 1

enum {
	CMD_SCAN_MATRIX_REQ = 0x0F,
	CMD_SCAN_MATRIX_RESP,
	CMD_STDIO_PUTS
};

enum { SLAVE, MASTER };
enum { LEFT, RIGHT };

static void uart_tx_init(void);
static void uart_rx_init(void);
static void uart_full_init(void);

static void uart_leave_rx(void);
static void uart_enter_rx(void);

static bool uart_await_rx(void);
static bool uart_await_tx(void);

static uint8_t uart_rx_byte(void);
static void uart_tx_byte(uint8_t c);

static uint uart_recv(uint8_t *data, uint len);
static uint uart_send(const uint8_t *data, uint len);

static void irq_rx_cmd(uint8_t cmd);
static void irq_rx(void);

static uint uart_tx_sm;
static uint uart_tx_sm_offset;

static uint uart_rx_sm;
static uint uart_rx_sm_offset;

static bool scan_pending = false;

void
uart_tx_init(void)
{
	pio_sm_config config;

	uart_tx_sm = CLAIM_UNUSED_SM(pio0);
	uart_tx_sm_offset = pio_add_program(pio0, &uart_tx_program);

	config = uart_tx_program_get_default_config(uart_tx_sm_offset);
	sm_config_set_out_shift(&config, true, false, 32);
	sm_config_set_out_pins(&config, UART_PIN, 1);
	sm_config_set_sideset_pins(&config, UART_PIN);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
	sm_config_set_clkdiv(&config,
		((float) clock_get_hz(clk_sys)) / (8 * UART_BAUD));

	pio_sm_init(pio0, uart_tx_sm, uart_tx_sm_offset, &config);
	pio_sm_set_enabled(pio0, uart_tx_sm, false);
}

void
uart_rx_init(void)
{
	pio_sm_config config;

	uart_rx_sm = CLAIM_UNUSED_SM(pio0);
	uart_rx_sm_offset = pio_add_program(pio0, &uart_rx_program);

	config = uart_rx_program_get_default_config(uart_rx_sm_offset);
	sm_config_set_in_pins(&config, UART_PIN);
	sm_config_set_jmp_pin(&config, UART_PIN);
	sm_config_set_in_shift(&config, true, false, 32);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);
	sm_config_set_clkdiv(&config,
		((float) clock_get_hz(clk_sys)) / (8 * UART_BAUD));

	pio_sm_init(pio0, uart_rx_sm, uart_rx_sm_offset, &config);
	pio_sm_set_enabled(pio0, uart_rx_sm, false);
}

void
uart_full_init(void)
{
	pio_sm_set_pins_with_mask(pio0, uart_tx_sm, 0U, 1U << UART_PIN);
	pio_sm_set_consecutive_pindirs(pio0, uart_tx_sm, UART_PIN, 1, true);

	pio_gpio_init(pio0, UART_PIN);
	gpio_pull_up(UART_PIN);
	gpio_set_slew_rate(UART_PIN, GPIO_SLEW_RATE_FAST);
	/* 1 => set INPUT and pull line HIGH from pullup
	 * 0 => set OUTPUT and pull line LOW from signal */
	gpio_set_oeover(UART_PIN, GPIO_OVERRIDE_INVERT);

	uart_rx_init();
	uart_tx_init();

	pio_set_irq0_source_enabled(pio0,
		pis_sm0_rx_fifo_not_empty + uart_rx_sm, false);
	pio_set_irq0_source_enabled(pio0,
		pis_sm0_tx_fifo_not_full + uart_tx_sm, false);
	pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);
	pio_set_irq0_source_enabled(pio0, pis_interrupt1, true);

	irq_set_priority(PIO0_IRQ_0, PICO_HIGHEST_IRQ_PRIORITY);
	irq_set_exclusive_handler(PIO0_IRQ_0, irq_rx);
	irq_set_enabled(PIO0_IRQ_0, true);

	pio_sm_set_enabled(pio0, uart_tx_sm, true);
	uart_enter_rx();
}

void
uart_leave_rx(void)
{
	irq_set_enabled(USBCTRL_IRQ, false);
	pio_sm_set_enabled(pio0, uart_rx_sm, false);

	/* because of OE override pindir true = INPUT (!) */
	pio_sm_set_consecutive_pindirs(pio0, uart_tx_sm, UART_PIN, 1, true);

	/* drive LOW with high drive-current for steep falling edges */
	pio_sm_set_pins_with_mask(pio0, uart_tx_sm, 0U, 1 << UART_PIN);
	gpio_set_drive_strength(UART_PIN, GPIO_DRIVE_STRENGTH_12MA);

	pio_sm_restart(pio0, uart_tx_sm);
	pio_sm_set_enabled(pio0, uart_tx_sm, true);
}

void
uart_enter_rx(void)
{
	/* wait for tx fifo to empty and final byte to transmit
	 * + extra max. 1 start + 8 data + 1 stop + 1 par bits */
	while (!pio_sm_is_tx_fifo_empty(pio0, uart_tx_sm));
	sleep_us(1000000U * 11 / UART_BAUD);
	pio_sm_set_enabled(pio0, uart_tx_sm, false);

	/* pull HIGH with low drive-current for steeper rising edge */
	gpio_set_drive_strength(UART_PIN, GPIO_DRIVE_STRENGTH_2MA);
	pio_sm_set_pins_with_mask(pio0, uart_tx_sm, ~0U, 1 << UART_PIN);

	/* because of OE override pindir false = OUTPUT (!) */
	pio_sm_set_consecutive_pindirs(pio0, uart_tx_sm, UART_PIN, 1, false);

	pio_sm_set_enabled(pio0, uart_rx_sm, true);
	irq_set_enabled(USBCTRL_IRQ, true);
}

bool
uart_await_rx(void)
{
	uint32_t start_ms;
	bool empty;

	start_ms = board_millis();
	do {
		empty = pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm);
		if (!empty) break;
		tud_task();
	} while (board_millis() < start_ms + UART_TIMEOUT);

	return !empty;
}

bool
uart_await_tx(void)
{
	uint32_t start_ms;
	bool full;

	start_ms = board_millis();
	do {
		full = pio_sm_is_tx_fifo_full(pio0, uart_tx_sm);
		if (!full) break;
		tud_task();
	} while (board_millis() < start_ms + UART_TIMEOUT);

	return !full;
}

uint8_t
uart_rx_byte(void)
{
	return *(uint8_t*)((uintptr_t)&pio0->rxf[uart_rx_sm] + 3);
}

void
uart_tx_byte(uint8_t c)
{
	pio_sm_put(pio0, uart_tx_sm, c);
}

uint
uart_recv(uint8_t *data, uint len)
{
	uint recv;

	for (recv = 0; recv < len; recv++) {
		if (pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm)) {
			if (!uart_await_rx()) break;
		}
		*data++ = uart_rx_byte();
	}

	return recv;
}

uint
uart_send(const uint8_t *data, uint len)
{
	uint sent;

	uart_leave_rx();
	for (sent = 0; sent < len; sent++) {
		if (pio_sm_is_tx_fifo_full(pio0, uart_tx_sm)) {
			if (!uart_await_tx()) break;
		}
		uart_tx_byte(*data++);
	}
	uart_enter_rx();

	return sent;
}

void
irq_rx_cmd(uint8_t cmd)
{
	uint8_t buf[64];

	switch (cmd) {
	case CMD_SCAN_MATRIX_REQ:
		if (SPLIT_ROLE != SLAVE)
			break;
		scan_pending = true;
		return;
	case CMD_SCAN_MATRIX_RESP:
		if (SPLIT_ROLE != MASTER)
			break;
		scan_pending = false;
		return;
	case CMD_STDIO_PUTS:
		if (SPLIT_ROLE != MASTER)
			break;
		memset(buf, 0, sizeof(buf));
		uart_recv(buf, sizeof(buf)-1);
		printf("SLAVE: %s\n", buf);
		return;
	}

	WARN("Unexpected uart cmd: %i", cmd);
}

void
irq_rx(void)
{
	uint8_t cmd;

	(void) cmd;
	(void) irq_rx_cmd;

	if (pio_interrupt_get(pio0, 0)) {
		DEBUG("RX ERR");
		pio_interrupt_clear(pio0, 0);
	}

	//DEBUG("UART IRQ");
	// while (!pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm))
	// 	uart_rx_byte();
	// ASSERT(1 == 0);

	while (!pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm)) {
		cmd = uart_rx_byte();
		DEBUG("UART RX CMD %i\n", cmd);
		irq_rx_cmd(cmd);
	}
}

void
split_init(void)
{
	uart_full_init();
}

void
split_test(void)
{
	pio_sm_set_pins_with_mask(pio0, uart_tx_sm, 0U, 1U << UART_PIN);
	pio_sm_set_consecutive_pindirs(pio0, uart_tx_sm, UART_PIN, 1, false);

	pio_gpio_init(pio0, UART_PIN);
	gpio_pull_up(UART_PIN);
	gpio_set_slew_rate(UART_PIN, GPIO_SLEW_RATE_FAST);

	/* 1 => set INPUT and pull line HIGH from pullup
	 * 0 => set OUTPUT and pull line LOW from signal */
	gpio_set_oeover(UART_PIN, GPIO_OVERRIDE_INVERT);
}

void
split_task(void)
{
	uint32_t start_ms;
	uint8_t cmd;

	// if (!uart_await_tx())
	// 	return;

	// sleep_us(100);
	// uart_leave_rx();
	// uart_tx_byte(0xAA);
	// uart_enter_rx();

	// return;

	if (SPLIT_ROLE == MASTER) {
		scan_pending = true;
		cmd = CMD_SCAN_MATRIX_REQ;
		ASSERT(uart_send(&cmd, 1) == 1);
		scan_matrix(); /* scan our side in parallel */
		start_ms = board_millis();
		while (scan_pending && board_millis() < start_ms + 50)
			tud_task();
		if (scan_pending) WARN("Slave matrix scan timeout");
		else DEBUG("Slave matrix scan success");
		scan_pending = false;
	} else {
		if (!uart_await_rx())
			return;
		cmd = uart_rx_byte();
		DEBUG("GOT RX: %i", cmd);
		if (scan_pending) {
			scan_matrix();
			cmd = CMD_SCAN_MATRIX_RESP;
			ASSERT(uart_send(&cmd, 1) == 1);
			scan_pending = false;
		}
	} 
}
