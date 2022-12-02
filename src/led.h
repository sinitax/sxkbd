#pragma once

#include <stdint.h>
#include <stdbool.h>

enum {
	LED_OFF,
	LED_ON,
	LED_BLINK
};

extern int led_mode;
extern bool led_reset;
extern uint32_t led_blink_ms;
extern uint32_t led_rgb;

void led_init(void);
void led_task(void);
