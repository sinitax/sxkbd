#pragma once

#include "hardware/pio.h"
#include "hardware/gpio.h"

#include <stdbool.h>

struct ws2812 {
	PIO pio;
	uint sm;
	uint pin;
	bool init;
};

void ws2812_init(struct ws2812 *pix, PIO pio, uint pin);
void ws2812_put(struct ws2812 *pix, uint32_t rgb);
