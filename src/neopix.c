#include "neopix.h"
#include "ws2812.pio.h"
#include "util.h"

#include "hardware/pio.h"

void
neopix_init(struct neopix *pix, uint pin)
{
	uint offset;
	int sm;

	pix->pio = pio0;
	sm = pio_claim_unused_sm(pix->pio, true);
	ASSERT(sm >= 0);
	pix->sm = (uint) sm;
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
