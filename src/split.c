#include "split.h"
#include "class/cdc/cdc_device.h"
#include "util.h"
#include "matrix.h"

#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/time.h"
#include "bsp/board.h"
#include "tusb.h"
#include <stdint.h>

#define UART_TIMEOUT 20
#define UART_BAUD 115200

/* same pin since half-duplex */
#define UART_TX_PIN 0
#define UART_RX_PIN 0

enum {
	CMD_SCAN_MATRIX_REQ = 0x80,
	CMD_SCAN_MATRIX_RESP,
	CMD_STDIO_PUTS
};

enum { SLAVE, MASTER };
enum { LEFT, RIGHT };

static void uart_tx_init(void);
static void uart_rx_init(void);

static void uart_enter_rx(void);
static void uart_leave_rx(void);

static int uart_sync_rx(void);
static int uart_sync_tx(void);

static uint8_t uart_in_byte(void);
static void uart_out_byte(uint8_t c);

static uint uart_recv(uint8_t *data, uint len);
static uint uart_send(const uint8_t *data, uint len);

static void irq_rx_cmd(uint8_t cmd);
static void irq_rx(void);

static const uint16_t uart_tx_program_asm[] = {
	        //     .wrap_target
	0x9fa0, //  0: pull   block           side 1 [7]
	0xf727, //  1: set    x, 7            side 0 [7]
	0x6081, //  2: out    pindirs, 1
	0x0642, //  3: jmp    x--, 2                 [6]
	        //     .wrap
};

static const uint16_t uart_rx_program_asm[] = {
	        //     .wrap_target
	0x2020, //  0: wait   0 pin, 0
	0xea27, //  1: set    x, 7                   [10]
	0x4001, //  2: in     pins, 1
	0x0642, //  3: jmp    x--, 2                 [6]
	0x00c8, //  4: jmp    pin, 8
	0xc020, //  5: irq    wait 0
	0x20a0, //  6: wait   1 pin, 0
	0x0000, //  7: jmp    0
	0x8020, //  8: push   block
	        //     .wrap
};

static const pio_program_t uart_tx_program = {
	.instructions = uart_tx_program_asm,
	.length = 4,
	.origin = -1,
};

static const pio_program_t uart_rx_program = {
	.instructions = uart_rx_program_asm,
	.length = 9,
	.origin = -1,
};

static uint uart_tx_sm;
static uint uart_rx_sm;

static bool scan_pending = false;

void
uart_tx_init(void)
{
	pio_sm_config config;
	uint offset;
	int sm;

	sm = pio_claim_unused_sm(pio0, true);
	ASSERT(sm >= 0);
	uart_tx_sm = (uint) sm;

	offset = pio_add_program(pio0, &uart_tx_program);
	pio_sm_set_pins_with_mask(pio0, uart_tx_sm, 0, 1 << UART_TX_PIN);
	pio_sm_set_consecutive_pindirs(pio0,
		uart_tx_sm, UART_TX_PIN, 1, true);

	config = pio_get_default_sm_config();
	sm_config_set_wrap(&config, offset,
		offset + ARRLEN(uart_tx_program_asm) - 1);
	sm_config_set_sideset(&config, 2, true, true);
	sm_config_set_out_shift(&config, true, false, 32);
	sm_config_set_out_pins(&config, UART_TX_PIN, 1);
	sm_config_set_sideset_pins(&config, UART_TX_PIN);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
	sm_config_set_clkdiv(&config,
		(float) clock_get_hz(clk_sys) / (8 * UART_BAUD));

	pio_sm_init(pio0, uart_tx_sm, offset, &config);
	pio_sm_set_enabled(pio0, uart_tx_sm, true);
}

void
uart_rx_init(void)
{
	pio_sm_config config;
	uint offset;
	int sm;

	sm = pio_claim_unused_sm(pio0, true);
	ASSERT(sm >= 0);
	uart_rx_sm = (uint) sm;

	offset = pio_add_program(pio0, &uart_rx_program);

	config = pio_get_default_sm_config();
	sm_config_set_wrap(&config, offset,
		offset + ARRLEN(uart_rx_program_asm) - 1);
	sm_config_set_in_pins(&config, UART_RX_PIN);
	sm_config_set_jmp_pin(&config, UART_RX_PIN);
	sm_config_set_in_shift(&config, true, false, 32);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);
	sm_config_set_clkdiv(&config,
		(float) clock_get_hz(clk_sys) / (8 * UART_BAUD));

	pio_sm_init(pio0, uart_rx_sm, offset, &config);
	pio_sm_set_enabled(pio0, uart_rx_sm, true);
}

void
uart_enter_rx(void)
{
	while (!pio_sm_is_tx_fifo_empty(pio0, uart_tx_sm));
	/* even after fifo is empty, we still need to wait until last byte
	 * = max (1 start + 8 data + 1 stop + 1 par) is fully transmitted */
	sleep_us(1000000U * 11 / UART_BAUD);

	pio_sm_set_enabled(pio0, uart_tx_sm, false);
	gpio_set_drive_strength(UART_TX_PIN, GPIO_DRIVE_STRENGTH_2MA);
	pio_sm_set_pins_with_mask(pio0,
		uart_tx_sm, 1U << UART_TX_PIN, 1U << UART_TX_PIN);
	pio_sm_set_consecutive_pindirs(pio0,
		uart_tx_sm, UART_TX_PIN, 1, false);
	pio_sm_set_enabled(pio0, uart_rx_sm, true);
}

void
uart_leave_rx(void)
{
	/* disable rx to not receive when we send */
	pio_sm_set_enabled(pio0, uart_rx_sm, false);
	pio_sm_set_consecutive_pindirs(pio0,
		uart_tx_sm, UART_TX_PIN, 1, true);
	pio_sm_set_pins_with_mask(UART_TX_PIN,
		uart_tx_sm, 0, 1U << UART_TX_PIN);
	gpio_set_drive_strength(UART_TX_PIN, GPIO_DRIVE_STRENGTH_12MA);
	pio_sm_restart(pio0, uart_tx_sm);
	pio_sm_set_enabled(pio0, uart_tx_sm, true);
}

int
uart_sync_rx(void)
{
	uint32_t start_ms;
	bool empty;

	start_ms = board_millis();
	do {
		empty = pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm);
		if (!empty) break;
		tud_task();
	} while (board_millis() < start_ms + UART_TIMEOUT);

	return empty;
}

int
uart_sync_tx(void)
{
	uint32_t start_ms;
	bool full;

	start_ms = board_millis();
	do {
		full = pio_sm_is_tx_fifo_full(pio0, uart_tx_sm);
		if (!full) break;
		tud_task();
	} while (board_millis() < start_ms + UART_TIMEOUT);

	return full;
}

uint8_t
uart_in_byte(void)
{
	return *(uint8_t*)((uintptr_t)&pio0->rxf[uart_rx_sm] + 3);
}

void
uart_out_byte(uint8_t c)
{
	pio_sm_put(pio0, uart_tx_sm, c);
}

uint
uart_recv(uint8_t *data, uint len)
{
	uint recv;

	recv = 0;
	while (recv < len) {
		if (pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm)) {
			if (uart_sync_rx()) break;
		}
		*data++ = uart_in_byte();
		recv++;
	}

	return recv;
}

uint
uart_send(const uint8_t *data, uint len)
{
	uint sent;

	uart_leave_rx();
	sent = 0;
	while (sent < len) {
		if (pio_sm_is_tx_fifo_full(pio0, uart_tx_sm)) {
			if (uart_sync_tx()) break;
		}
		uart_out_byte(*data++);
		sent++;
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

	DEBUG("UART IRQ\n");
	while (!pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm)) {
		cmd = uart_in_byte();
		DEBUG("UART RX CMD %i\n", cmd);
		irq_rx_cmd(cmd);
	}
}

void
split_init(void)
{
	uart_tx_init();
	uart_rx_init();

	pio_set_irq0_source_enabled(pio0,
		pis_sm0_rx_fifo_not_empty + uart_rx_sm, false);
	pio_set_irq0_source_enabled(pio0,
		pis_sm0_tx_fifo_not_full + uart_tx_sm, false);
	pio_set_irq0_source_enabled(pio0, pis_interrupt0, false);

	irq_set_priority(PIO0_IRQ_0, PICO_HIGHEST_IRQ_PRIORITY);
	irq_set_exclusive_handler(PIO0_IRQ_0, irq_rx);
	irq_set_enabled(PIO0_IRQ_0, true);

	uart_enter_rx();
}

void
split_task(void)
{
	uint32_t start_ms;
	uint8_t cmd;

	if (SPLIT_ROLE == SLAVE) {
		if (scan_pending) {
			scan_matrix();
			cmd = CMD_SCAN_MATRIX_RESP;
			ASSERT(uart_send(&cmd, 1));
			scan_pending = false;
		}
	} else if (SPLIT_ROLE == MASTER) {
		scan_pending = true;
		cmd = CMD_SCAN_MATRIX_REQ;
		ASSERT(uart_send(&cmd, 1));
		scan_matrix(); /* scan our side in parallel */
		start_ms = board_millis();
		while (scan_pending && board_millis() < start_ms + 50)
			tud_task();
		if (scan_pending) WARN("Slave matrix scan timeout");
		else DEBUG("Slave matrix scan success");
		scan_pending = false;
	}
}
