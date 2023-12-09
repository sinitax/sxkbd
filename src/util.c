#include "util.h"

#include "split.h"
#include "board.h"
#include "class/cdc/cdc_device.h"
#include "led.h"
#include "ws2812.h"

#include "pico/stdio.h"
#include "tusb.h"
#include "bsp/board.h"
#include "tusb_config.h"

#include <stdio.h>

char warnlog[256];
int log_level_min = LOG_INFO;
int log_group_mask = LOG_ALL;

static void
__attribute__((format(printf, 1, 0)))
panic_task(const char *fmtstr, va_list ap, uint32_t sleep_ms)
{
	static uint32_t start_ms = 0;
	va_list cpy;

	if (!tud_cdc_connected())
		return;

	if (!start_ms) start_ms = board_millis();

	if (board_millis() < start_ms + sleep_ms)
		return;

	va_copy(cpy, ap);
	vprintf(fmtstr, cpy);
	printf("\n");
	tud_cdc_write_flush();

	start_ms += sleep_ms;
}

void
__attribute__ ((format (printf, 3, 4)))
stdio_log(int facility, int level, const char *fmtstr, ...)
{
	va_list ap, cpy;

	if (!(facility & log_group_mask))
		return;

	if (level < log_level_min)
		return;

	if (level == LOG_WARN) {
		led_start_blip(HARD_RED, 100);
		va_copy(cpy, ap);
		va_start(cpy, fmtstr);
		vsnprintf(warnlog, sizeof(warnlog), fmtstr, cpy);
		va_end(cpy);

		if (split_role == SLAVE)
			split_warn_master(warnlog);
	}

	if (!tud_cdc_connected())
		return;

	va_start(ap, fmtstr);
	vprintf(fmtstr, ap);
	va_end(ap);
	printf("\n");
	tud_cdc_write_flush();
}

void
__attribute__((format(printf, 3, 4)))
blink_panic(uint32_t blink_ms, uint32_t rgb, const char *fmtstr, ...)
{
	va_list ap;

	led_blink_ms = blink_ms;
	led_rgb = rgb;
	led_mode = LED_BLINK;

	va_start(ap, fmtstr);
	while (1) {
		tud_task();
		panic_task(fmtstr, ap, 1000);
		led_task();
	}
	va_end(ap);
}
