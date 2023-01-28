#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SOFT_WHITE  0x404040
#define SOFT_YELLOW 0x404000
#define SOFT_PURPLE 0x400040
#define HARD_WHITE  0xFFFFFF
#define HARD_YELLOW 0xFF00FF
#define HARD_RED    0xFF0000

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
void led_blip(uint32_t rgb);
