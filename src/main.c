#include "board.h"
#include "usb_stdio.h"
#include "matrix.h"
#include "keysym.h"
#include "split.h"
#include "led.h"
#include "hid.h"
#include "keymap.h"
#include "ws2812.h"
#include "util.h"

#include "hardware/gpio.h"
#include "class/cdc/cdc_device.h"
#include "class/hid/hid.h"
#include "device/usbd.h"
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "pico/time.h"
#include "tusb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool send_hid_report(int id);
void cdc_task(void);

const uint32_t **keymap_layers = keymap_layers_de;

int
main(void)
{
	board_init();
	tud_init(BOARD_TUD_RHPORT);
	usb_stdio_init();
	led_init();
	matrix_init();
	split_init();
	//hid_init();

	while (true) {
		tud_task();
		cdc_task();
		led_task();
		split_task();
		//hid_task();
		send_hid_report(REPORT_ID_MIN);
	}

	return 0;
}

void
tud_mount_cb(void)
{
	led_rgb = WS2812_U32RGB(100, 0, 100);
	led_mode = LED_ON;
	led_reset = true;
}

void
tud_umount_cb(void)
{
	led_blink_ms = 500;
	led_rgb = WS2812_U32RGB(100, 100, 100);
	led_mode = LED_BLINK;
	led_reset = true;
}

void
tud_suspend_cb(bool remote_wakeup_en)
{
	led_rgb = WS2812_U32RGB(100, 100, 100);
	led_mode = LED_ON;
	led_reset = true;
}

void
tud_resume_cb(void)
{
	led_rgb = WS2812_U32RGB(100, 0, 100);
	led_mode = LED_ON;
	led_reset = true;
}

void
tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
}

void
tud_cdc_rx_cb(uint8_t itf)
{
}

uint16_t
tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
	hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	return 0;
}

void
tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
	hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

void
tud_hid_report_complete_cb(uint8_t instance,
	uint8_t const *report, uint8_t len)
{
	uint8_t id;

	for (id = report[0] + 1; id < REPORT_ID_MAX; id++) {
		if (send_hid_report(id))
			break;
	}
}

bool
send_keyboard_report(void)
{
	static bool cleared = true;
	uint8_t report[6] = { 0 };
	bool any;

	any = hid_gen_report(report);

	if (any) {
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, report);
		cleared = false;
		return true;
	} else if (!cleared) {
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
		cleared = true;
		return true;
	}

	return false;
}

bool
send_mouse_report(bool state)
{
	if (state) {
		tud_hid_mouse_report(REPORT_ID_MOUSE, 0, 10, 10, 0, 0);
		return true;
	}

	return false;
}

bool
send_consumer_control_report(bool state)
{
	static bool cleared = true;
	uint16_t report;

	if (state) {
		report = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
		tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report, 2);
		cleared = false;
		return true;
	} else if (!cleared) {
		report = 0;
		tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report, 2);
		cleared = true;
		return true;
	}

	return false;
}

bool
send_hid_report(int id)
{
	if (!tud_hid_ready()) return false;

	switch (id) {
	case REPORT_ID_KEYBOARD:
		return send_keyboard_report();
	case REPORT_ID_MOUSE:
		return send_mouse_report(false);
	case REPORT_ID_CONSUMER_CONTROL:
		return send_consumer_control_report(false);
	}

	return false;
}

void
process_cmd(char *cmd)
{
	char *arg, *tok;

	tok = strchr(cmd, ' ');
	if (tok) {
		*tok = '\0';
		arg = tok + 1;
	} else {
		arg = cmd + strlen(cmd);
	}

	if (!strcmp(cmd, "log")) {
		if (!strcmp(arg, "")) {
			printf("Levels: debug, info, warn, err\n");
		} else if (!strcmp(arg, "debug")) {
			loglevel = LOG_DEBUG;
		} else if (!strcmp(arg, "info")) {
			loglevel = LOG_INFO;
		} else if (!strcmp(arg, "warn")) {
			loglevel = LOG_WARN;
		} else if (!strcmp(arg, "warn")) {
			loglevel = LOG_ERR;
		} else {
			printf("Invalid log level: %s\n", arg);
		}
	} else {
		printf("Invalid command: %s\n", cmd);
	}
}

void
cdc_task(void)
{
	static char cmdbuf[256];
	static int cmdlen = 0;
	char c;

	do {
		if (cmdlen)
			tud_task();

		if (tud_cdc_connected() && tud_cdc_available()
				&& tud_cdc_read(&c, 1)) {
			if (c == '\r' || c == '\n') {
				printf("\n");
				tud_cdc_write_flush();
				if (cmdlen) {
					cmdbuf[cmdlen] = 0;
					process_cmd(cmdbuf);
					cmdlen = 0;
				}
			} else if (c == 4) {
				printf("ALIVE!\n");
			} else if (cmdlen == ARRLEN(cmdbuf)) {
				printf("\n--- cmd too long ---\n");
				cmdlen = 0;
			} else {
				printf("%c", c);
				tud_cdc_write_flush();
				cmdbuf[cmdlen++] = c;
			}
		}
	} while (cmdlen);
}
