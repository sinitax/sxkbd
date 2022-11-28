#include "util.h"
#include "board.h"
#include "neopix.h"

#include "tusb.h"
#include "bsp/board.h"
#include "tusb_config.h"

#include <stdio.h>

int loglevel = LOG_DEBUG;

static void
__attribute__((format(printf, 1, 0)))
panic_task(const char *fmtstr, va_list ap)
{
	va_list cpy;
	char c;

	if (!tud_cdc_available())
		return;

	if (!tud_cdc_read(&c, 1))
		return;

	va_copy(cpy, ap);
	vprintf(fmtstr, cpy);
	printf("\n\r");
}

static void
blink_task(struct neopix *pix, uint32_t blink_ms)
{
	static uint32_t start_ms = 0;
	static bool led_state = false;

	if (!start_ms) start_ms = board_millis();

	if (board_millis() - start_ms < blink_ms)
		return;

	neopix_put(pix, neopix_u32rgb(255 * led_state, 0, 0));

	led_state ^= true;
	start_ms += blink_ms;
}

void
__attribute__ ((format (printf, 2, 3)))
stdio_log(int level, const char *fmtstr, ...)
{
	va_list ap;

	if (!tud_cdc_available())
		return;

	if (level > loglevel)
		return;

	va_start(ap, fmtstr);
	vprintf(fmtstr, ap);
	va_end(ap);
	printf("\n\r");
}

void
__attribute__((format(printf, 1, 2)))
blink_panic(const char *fmtstr, ...)
{
	va_list ap;

	va_start(ap, fmtstr);

	if (!onboard_led.init)
		neopix_init(&onboard_led, pio0, 0, ONBOARD_LED_PIN);

	while (1) {
		tud_task();
		panic_task(fmtstr, ap);
		blink_task(&onboard_led, 200);
	}

	va_end(ap);
}
