#include "ws2812.h"
#include "ws2812.pio.h"
#include "util.h"

#include "hardware/pio.h"
#include "hardware/clocks.h"

#define CYCLES_PER_BIT (ws2812_T1 + ws2812_T2 + ws2812_T3)

void
ws2812_init(struct ws2812 *pix, PIO pio, uint pin)
{
	pio_sm_config config;
	uint offset;
	uint sm;

	pix->pio = pio;
	pix->pin = pin;

	sm = CLAIM_UNUSED_SM(pio);
	pix->sm = sm;

	pio_gpio_init(pio, pin);
	pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

	offset = pio_add_program(pix->pio, &ws2812_program);
	config = ws2812_program_get_default_config(offset);
	sm_config_set_sideset_pins(&config, pin);
	sm_config_set_out_shift(&config, false, true, 24);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
	sm_config_set_clkdiv(&config,
		(float) clock_get_hz(clk_sys) / (800000 * CYCLES_PER_BIT));

	pio_sm_init(pio, sm, offset, &config);
	pio_sm_set_enabled(pio, sm, true);

	pix->init = true;
}

void
ws2812_put(struct ws2812 *pix, uint32_t rgb) {
	pio_sm_put_blocking(pix->pio, pix->sm, rgb << 8u);
}
