#include "split.h"
#include "util.h"
#include "keymat.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

#include "device/usbd.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/regs/intctrl.h"
#include "hardware/clocks.h"

#include <stdint.h>

#define UART_AWAIT_TIMEOUT_US 1000
#define UART_RECV_TIMEOUT_US 200
#define UART_SEND_TIMEOUT_US 200

#define UART_BAUD 115200

#define CMD_START 0x8a

#if SPLIT_SIDE == LEFT
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#elif SPLIT_SIDE == RIGHT
#define UART_TX_PIN 1
#define UART_RX_PIN 0
#else
#error "SPLIT_SIDE not set"
#endif

enum {
	CMD_SCAN_KEYMAT_REQ = 0x80,
	CMD_SCAN_KEYMAT_RESP,
	CMD_SLAVE_WARN
};

static uint uart_tx_sm;
static uint uart_tx_sm_offset;

static uint uart_rx_sm;
static uint uart_rx_sm_offset;

static uint32_t halfmat;

int split_role;

static void
irq_rx(void)
{
	if (pio_interrupt_get(pio0, 0)) {
		DEBUG(LOG_SPLIT, "UART RX ERR");
		pio_interrupt_clear(pio0, 0);
	}
}

static void
uart_tx_sm_init(void)
{
	pio_sm_config config;

	uart_tx_sm = claim_unused_sm(pio0);
	uart_tx_sm_offset = pio_add_program(pio0, &uart_tx_program);

	config = uart_tx_program_get_default_config(uart_tx_sm_offset);
	sm_config_set_out_shift(&config, true, false, 32);
	sm_config_set_out_pins(&config, UART_TX_PIN, 1);
	sm_config_set_sideset_pins(&config, UART_TX_PIN);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
	sm_config_set_clkdiv(&config,
		((float) clock_get_hz(clk_sys)) / (8 * UART_BAUD));

	pio_sm_init(pio0, uart_tx_sm, uart_tx_sm_offset, &config);
	pio_sm_set_enabled(pio0, uart_tx_sm, false);
}

static void
uart_rx_sm_init(void)
{
	pio_sm_config config;

	uart_rx_sm = claim_unused_sm(pio0);
	uart_rx_sm_offset = pio_add_program(pio0, &uart_rx_program);

	config = uart_rx_program_get_default_config(uart_rx_sm_offset);
	sm_config_set_in_pins(&config, UART_RX_PIN);
	sm_config_set_jmp_pin(&config, UART_RX_PIN);
	sm_config_set_in_shift(&config, true, false, 32);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);
	sm_config_set_clkdiv(&config,
		((float) clock_get_hz(clk_sys)) / (8 * UART_BAUD));

	pio_sm_init(pio0, uart_rx_sm, uart_rx_sm_offset, &config);
	pio_sm_set_enabled(pio0, uart_rx_sm, false);
}

static void
uart_full_init(void)
{
	pio_gpio_init(pio0, UART_RX_PIN);
	pio_sm_set_pins_with_mask(pio0, uart_rx_sm, 1U, 1U << UART_RX_PIN);
	pio_sm_set_consecutive_pindirs(pio0, uart_rx_sm, UART_RX_PIN, 1, false);

	pio_gpio_init(pio0, UART_TX_PIN);
	gpio_set_slew_rate(UART_TX_PIN, GPIO_SLEW_RATE_FAST);
	gpio_set_drive_strength(UART_TX_PIN, GPIO_DRIVE_STRENGTH_4MA);
	pio_sm_set_pins_with_mask(pio0, uart_tx_sm, 1U, 1U << UART_TX_PIN);
	pio_sm_set_consecutive_pindirs(pio0, uart_tx_sm, UART_TX_PIN, 1, true);

	uart_rx_sm_init();
	uart_tx_sm_init();

	pio_set_irq0_source_enabled(pio0,
		pis_sm0_rx_fifo_not_empty + uart_rx_sm, false);
	pio_set_irq0_source_enabled(pio0,
		pis_sm0_tx_fifo_not_full + uart_tx_sm, false);
	pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);

	irq_set_priority(PIO0_IRQ_0, PICO_HIGHEST_IRQ_PRIORITY);
	irq_set_exclusive_handler(PIO0_IRQ_0, irq_rx);
	irq_set_enabled(PIO0_IRQ_0, true);

	pio_sm_set_enabled(pio0, uart_rx_sm, true);
	pio_sm_set_enabled(pio0, uart_tx_sm, true);
}

static bool
uart_await_rx(uint64_t timeout_us)
{
	uint64_t start_us;
	bool empty;

	if (!pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm))
		return true;

	start_us = board_micros();
	do {
		tud_task();
		empty = pio_sm_is_rx_fifo_empty(pio0, uart_rx_sm);
	} while (empty && board_micros() < start_us + timeout_us);

	return !empty;
}

static bool
uart_await_tx(uint64_t timeout_us)
{
	uint64_t start_us;
	bool full;

	if (!pio_sm_is_tx_fifo_full(pio0, uart_tx_sm))
		return true;

	start_us = board_micros();
	do {
		tud_task();
		full = pio_sm_is_tx_fifo_full(pio0, uart_tx_sm);
	} while (full && board_micros() < start_us + timeout_us);

	return !full;
}

static uint8_t
uart_rx_byte(void)
{
	return *(uint8_t*)((uintptr_t)&pio0->rxf[uart_rx_sm] + 3);
}

static void
uart_tx_byte(uint8_t c)
{
	pio_sm_put(pio0, uart_tx_sm, c);
}

static uint
uart_recv(uint8_t *data, uint len)
{
	uint recv;

	for (recv = 0; recv < len; recv++) {
		if (!uart_await_rx(UART_RECV_TIMEOUT_US))
			break;
		*data++ = uart_rx_byte();
	}

	return recv;
}

static uint
uart_recv_str(uint8_t *data, uint max)
{
	uint recv;

	for (recv = 0; recv < max; recv++) {
		if (!uart_await_rx(UART_RECV_TIMEOUT_US))
			break;
		*data++ = uart_rx_byte();
		if (!*data) break;
	}

	return recv;
}

static uint
uart_send(const uint8_t *data, uint len)
{
	uint sent;

	for (sent = 0; sent < len; sent++) {
		if (!uart_await_tx(UART_SEND_TIMEOUT_US))
			break;
		uart_tx_byte(*data++);
	}

	return sent;
}

static int
handle_cmd(void)
{
	static uint8_t msgbuf[128];
	uint8_t cmd, start;
	uint len;

	start = uart_rx_byte();
	if (start != CMD_START) {
		WARN(LOG_SPLIT, "Got bad start byte: %02u", start);
		return -1;
	}

	if (!uart_recv(&cmd, 1)) {
		WARN(LOG_SPLIT, "Got start byte without command");
		return -1;
	}

	switch (cmd) {
	case CMD_SCAN_KEYMAT_REQ:
		if (split_role != SLAVE) {
			WARN(LOG_SPLIT, "Got SCAN_KEYMAT_REQ as master");
			return -1;
		}
		break;
	case CMD_SCAN_KEYMAT_RESP:
		if (split_role != MASTER) {
			WARN(LOG_SPLIT, "Got SCAN_KEYMAT_RESP as slave");
			return -1;
		}
		if (uart_recv((uint8_t *) &halfmat, 4) != 4) {
			WARN(LOG_SPLIT, "Incomplete matrix received");
			return -1;
		}
		break;
	case CMD_SLAVE_WARN:
		if (split_role != MASTER) {
			WARN(LOG_SPLIT, "Got SLAVE_WARN as slave");
			return -1;
		}
		len = uart_recv_str(msgbuf, sizeof(msgbuf)-1);
		msgbuf[len] = '\0';
		WARN(LOG_SPLIT, "SLAVE: %s\n", msgbuf);
		break;
	default:
		WARN(LOG_SPLIT, "Unknown uart cmd: %i", cmd);
		return -1;
	}

	return cmd;
}

static bool
send_cmd(uint8_t cmd)
{
	uint8_t buf[2];

	buf[0] = CMD_START;
	buf[1] = cmd;

	return uart_send(buf, 2) == 2;
}

void
split_init(void)
{
	uart_full_init();
#ifdef SPLIT_ROLE
	split_role = SPLIT_ROLE;
#else
	split_role = SLAVE;
#endif
}

static void
split_task_master(void)
{
	int cmd;

	keymat_scan();

	if (uart_await_rx(UART_AWAIT_TIMEOUT_US)) {
		if ((cmd = handle_cmd()) == CMD_SCAN_KEYMAT_RESP) {
			keymat_decode_half(SPLIT_OPP(SPLIT_SIDE), halfmat);
		} else {
			WARN(LOG_SPLIT, "Got unexpected command %02x", cmd);
		}
	}

	keymat_debug();
}

static void
split_task_slave(void)
{
	if (keymat_scan()) {
		DEBUG(LOG_SPLIT, "Sending SCAN_KEYMAT_RESP");
		halfmat = keymat_encode_half(SPLIT_SIDE);
		if (!send_cmd(CMD_SCAN_KEYMAT_RESP)
				|| !uart_send((uint8_t *) &halfmat, 4)) {
			WARN(LOG_SPLIT,
				"UART send SCAN_KEYMAT_RESP failed");
		}
	}
}

void
split_task(void)
{
	if (split_role == MASTER) {
		split_task_master();
	} else {
		split_task_slave();
	}
}

void
split_warn_master(const char *msg)
{
	uint32_t len;

	if (!send_cmd(CMD_SLAVE_WARN)) {
		WARN(LOG_SPLIT, "UART send SLAVE_WARN failed");
		return;
	}

	len = strlen(msg) + 1;
	if (uart_send((const uint8_t *) msg, len) != len)
		WARN(LOG_SPLIT, "UART send warning failed");
}
