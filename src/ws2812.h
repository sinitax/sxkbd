#pragma once

#include "hardware/pio.h"
#include "hardware/gpio.h"

#include <stdbool.h>

#define _WS2812_U8(v, si, so) ((((uint32_t) (v) >> si) & 0xFF) << so)
#define WS2812_U32RGB(rgb) (_WS2812_U8(rgb, 16, 0) \
	| _WS2812_U8(rgb, 0, 8) | _WS2812_U8(rgb, 8, 16))

struct ws2812 {
	PIO pio;
	uint sm;
	uint pin;
	bool init;
};

void ws2812_init(struct ws2812 *pix, PIO pio, uint pin);
void ws2812_put(struct ws2812 *pix, uint32_t rgb);
