#include "board.h"
#include "usb_stdio.h"
#include "keymat.h"
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

void cdc_task(void);

static void
unassigned_init(void)
{
#ifdef BAD_GPIO_MITIGATION
#pragma message("Enabled bad gpio mitigation to swap gpio pins 5 & 9")
	const uint unassigned[] = { 8, 5, 23, 21 };
#else
	const uint unassigned[] = { 8, 9, 23, 21 };
#endif
	uint i;

	for (i = 0; i < ARRLEN(unassigned); i++) {
		gpio_init(unassigned[i]);
		gpio_set_dir(unassigned[i], GPIO_IN);
	}
}

int
main(void)
{
	uint32_t start, stop;

	board_init();
	tud_init(BOARD_TUD_RHPORT);
	usb_stdio_init();
	led_init();
	keymat_init();
	split_init();
	hid_init();
	unassigned_init();

	led_start_blip(HARD_WHITE, 500);

	start = board_millis();
	while (true) {
		tud_task();
		led_task();
		split_task();
		if (split_role == MASTER) {
			cdc_task();
			hid_task();
		}

		stop = board_millis();
		DEBUG(LOG_TIMING, "Main loop: %i ms", stop - start);
		start = stop;
	}

	return 0;
}

void
tud_mount_cb(void)
{
#ifndef SPLIT_ROLE
	split_role = MASTER;
#endif
	led_rgb = SOFT_PURPLE;
	led_mode = LED_ON;
	led_reset = true;
}

void
tud_umount_cb(void)
{
	led_blink_ms = 500;
	led_rgb = SOFT_WHITE;
	led_mode = LED_BLINK;
	led_reset = true;

	led_start_blip(HARD_WHITE, 500);
}

void
tud_suspend_cb(bool remote_wakeup_en)
{
#ifndef SPLIT_ROLE
	split_role = SLAVE;
#endif
	led_rgb = SOFT_WHITE;
	led_mode = LED_ON;
	led_reset = true;
}

void
tud_resume_cb(void)
{
	led_rgb = SOFT_PURPLE;
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

	if (!strcmp(cmd, "log-level")) {
		if (!strcmp(arg, "")) {
			printf("Levels: debug, info, warn, err\n");
		} else if (!strcmp(arg, "debug")) {
			log_level_min = LOG_DEBUG;
		} else if (!strcmp(arg, "info")) {
			log_level_min = LOG_INFO;
		} else if (!strcmp(arg, "warn")) {
			log_level_min = LOG_WARN;
		} else {
			printf("Invalid log level: %s\n", arg);
		}
	} else if (!strcmp(cmd, "log-groups")) {
		log_group_mask = 0;
		while (1) {
			tok = strchr(arg, ',');
			if (tok) *tok = '\0';
			if (!strcmp(arg, "all")) {
				log_group_mask |= LOG_ALL;
			} else if (!strcmp(arg, "keymat")) {
				log_group_mask |= LOG_KEYMAT;
			} else if (!strcmp(arg, "timing")) {
				log_group_mask |= LOG_TIMING;
			} else if (strspn(arg, "\t ") == strlen(arg)) {
				/* ignore */
			} else {
				printf("Invalid log facility: %s\n", arg);
			}
			if (!tok) break;
			arg = tok + 1;
		}
	} else if (!strcmp(cmd, "warn")) {
		if (*warnlog) {
			printf("Warning: %s\n", warnlog);
			*warnlog = '\0';
		} else {
			printf("No warnings logged\n");
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
