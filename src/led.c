#include "led.h"
#include "ws2812.h"
#include "util.h"

#include "bsp/board.h"

#define ONBOARD_LED_PIN 25

static struct ws2812 onboard_led;

int led_mode;
bool led_reset;
uint32_t led_blink_ms;
uint32_t led_rgb;

bool led_blip_reset;
bool led_blip;
uint32_t led_blip_ms;
uint32_t led_blip_rgb;

void
led_init(void)
{
	led_reset = true;
	led_mode = LED_ON;
	led_rgb = SOFT_WHITE;
	led_blip = false;
	led_blip_rgb = HARD_WHITE;
	ws2812_init(&onboard_led, pio0, ONBOARD_LED_PIN);
}

void
led_task(void)
{
	static uint32_t start_ms = 0;
	static bool state = false;

	if (led_blip_reset) {
		start_ms = board_millis();
		led_blip = true;
		ws2812_put(&onboard_led, led_blip_rgb);
		state = true;
		led_blip_reset = false;
	}

	if (led_blip) {
		if (state && board_millis() - start_ms > led_blip_ms) {
			ws2812_put(&onboard_led, led_rgb);
			state = false;
		} else if (board_millis() - start_ms > 2 * led_blip_ms) {
			led_blip = false;
			led_reset = true;
		}
	}

	if (led_blip) return;

	switch (led_mode) {
	case LED_OFF:
		if (led_reset) {
			led_rgb = 0;
			ws2812_put(&onboard_led, led_rgb);
		}
		break;
	case LED_ON:
		if (led_reset) {
			ws2812_put(&onboard_led, led_rgb);
		}
		break;
	case LED_BLINK:
		if (led_reset) {
			start_ms = board_millis();
			state = true;
			ws2812_put(&onboard_led, led_rgb);
		}

		if (board_millis() - start_ms < led_blink_ms)
			return;
		start_ms += led_blink_ms;

		state = !state;
		ws2812_put(&onboard_led, state ? led_rgb : 0);
		break;
	}

	led_reset = false;
}

void
led_set_mode(int mode, uint32_t rgb, uint32_t ms)
{
	led_mode = mode;
	led_rgb = rgb;
	led_blink_ms = ms;
}

void
led_start_blip(uint32_t rgb, uint32_t ms)
{
	if (led_blip) return;
	led_blip_rgb = rgb;
	led_blip_ms = ms;
	led_blip_reset = true;
}
