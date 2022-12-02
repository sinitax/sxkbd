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

void
led_init(void)
{
	led_reset = true;
	led_mode = LED_ON;
	led_rgb = WS2812_U32RGB(100, 100, 100);
	ws2812_init(&onboard_led, pio0, ONBOARD_LED_PIN);
}

void
led_task(void)
{
	static uint32_t start_ms = 0;
	static bool state = false;

	switch (led_mode) {
	case LED_OFF:
		if (led_reset)
			ws2812_put(&onboard_led, 0);
		break;
	case LED_ON:
		if (led_reset)
			ws2812_put(&onboard_led, led_rgb);
		break;
	case LED_BLINK:
		if (led_reset)
			start_ms = board_millis();

		if (board_millis() - start_ms < led_blink_ms)
			return;
		start_ms += led_blink_ms;

		state = !state;
		ws2812_put(&onboard_led, state ? led_rgb : 0);
		break;
	}

	led_reset = false;
}

