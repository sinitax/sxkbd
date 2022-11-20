#include "hardware/pio.h"
#include "hardware/gpio.h"

struct neopix {
	PIO pio;
	uint sm;
	uint pin;
};

void neopix_init(struct neopix *pix, PIO pio, uint sm, uint pin);
void neopix_put(struct neopix *pix, uint32_t rgb);

inline uint32_t
neopix_u32rgb(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t rgb;

	rgb = ((uint32_t) r) << 8;
	rgb |= ((uint32_t) g) << 16;
	rgb |= ((uint32_t) b) << 0;

	return rgb;
}
