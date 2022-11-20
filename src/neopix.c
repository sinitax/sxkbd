#include "hardware/pio.h"

#include "neopix.h"
#include "ws2812.pio.h"

void
neopix_init(struct neopix *pix, PIO pio, uint sm, uint pin)
{
	uint offset;

	pix->sm = sm;
	pix->pio = pio;
	pix->pin = pin;
	offset = pio_add_program(pix->pio, &ws2812_program);
	ws2812_program_init(pix->pio, pix->sm, offset,
		pix->pin, 800000, false);
	pix->init = true;
}

void
neopix_put(struct neopix *pix, uint32_t rgb) {
	pio_sm_put_blocking(pix->pio, pix->sm, rgb << 8u);
}
