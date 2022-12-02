#pragma once

#include "hardware/pio.h"
#include "hardware/gpio.h"

#include <stdbool.h>

#define _WS2812_U8(v, s) (((uint32_t) (v) & 0xFF) << s)
#define WS2812_U32RGB(r, g, b) \
	(_WS2812_U8(b, 0) | _WS2812_U8(r, 8) | _WS2812_U8(g, 16))

struct ws2812 {
	PIO pio;
	uint sm;
	uint pin;
	bool init;
};

void ws2812_init(struct ws2812 *pix, PIO pio, uint pin);
void ws2812_put(struct ws2812 *pix, uint32_t rgb);
