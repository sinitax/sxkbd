#include "split.h"
#include "util.h"
#include "matrix.h"

#include "hardware/gpio.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "bsp/board.h"
#include "tusb.h"

#define UART_TX_PIN 0
#define UART_RX_PIN 1

enum {
	CMD_SCAN_MATRIX_REQ = 0x80,
	CMD_SCAN_MATRIX_RESP,
	CMD_STDIO_PUTS
};

enum { SLAVE, MASTER };
enum { LEFT, RIGHT };

static void split_rx_cmd(uint8_t cmd);
static void split_rx(void);

static bool scan_pending = false;

void
split_rx_cmd(uint8_t cmd)
{
	char buf[64];
	uint8_t c;
	uint len;

	switch (cmd) {
	case CMD_SCAN_MATRIX_REQ:
		if (SPLIT_ROLE != SLAVE);
			break;
		scan_pending = true;
		return;
	case CMD_SCAN_MATRIX_RESP:
		if (SPLIT_ROLE != MASTER);
			break;
		scan_pending = false;
		return;
	case CMD_STDIO_PUTS:
		if (SPLIT_ROLE != MASTER);
			break;
		len = 0;
		while (uart_is_readable(uart0) && (c = uart_getc(uart0))) {
			if (len == ARRLEN(buf)) {
				tud_cdc_write(buf, len);
				buf[0] = c;
				len = 1;
			} else {
				buf[len++] = c;
			}
		}
		if (len) {
			tud_cdc_write(buf, len);
			tud_cdc_write_flush();
		}
		return;
	}

	WARN("Unexpected uart cmd: %i", cmd);
}

void
split_rx(void)
{
	while (uart_is_readable(uart0))
		split_rx_cmd(uart_getc(uart0));
}

void
split_init(void)
{
	uart_init(uart0, 2400);

	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	uart_set_baudrate(uart0, 115200);

	uart_set_hw_flow(uart0, false, false);

	uart_set_format(uart0, 8, 1, UART_PARITY_NONE);

	uart_set_fifo_enabled(uart0, true);

	irq_set_exclusive_handler(UART0_IRQ, split_rx);
	irq_set_enabled(UART0_IRQ, true);

	uart_set_irq_enables(uart0, true, false);
}

void
split_task(void)
{
	uint32_t start_ms;

	if (SPLIT_ROLE == SLAVE) {
		if (scan_pending) {
			scan_matrix();
			uart_putc_raw(uart0, CMD_SCAN_MATRIX_RESP);
			scan_pending = false;
		}
		uart_putc_raw(uart0, CMD_STDIO_PUTS);
		uart_putc_raw(uart0, '.');
		uart_putc_raw(uart0, 0);
	} else if (SPLIT_ROLE == MASTER) {
		scan_pending = true;
		uart_putc_raw(uart0, CMD_SCAN_MATRIX_REQ);
		scan_matrix(); /* scan our side in parallel */
		start_ms = board_millis();
		while (scan_pending && board_millis() < start_ms + 50)
			tud_task();
		if (scan_pending) WARN("Slave matrix scan timeout");
		else DEBUG("Slave matrix scan success");
		scan_pending = false;
	}
}
