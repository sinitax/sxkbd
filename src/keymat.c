#include "keymat.h"

#include "keymap.h"
#include "split.h"
#include "pico/types.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include <string.h>

static const uint keymat_row_pins[] = { 4, 5, 6, 7 };
static const uint keymat_col_pins[] = { 29, 28, 27, 26, 22, 20 };
static_assert(ARRLEN(keymat_row_pins) == KEY_ROWS_HALF);
static_assert(ARRLEN(keymat_col_pins) == KEY_COLS);

bool keymat_prev[KEY_ROWS][KEY_COLS];
bool keymat[KEY_ROWS][KEY_COLS];

void
keymat_init(void)
{
	uint x, y;

	for (x = 0; x < KEY_COLS; x++) {
		gpio_init(keymat_col_pins[x]);
		gpio_set_dir(keymat_col_pins[x], GPIO_IN);
		gpio_pull_up(keymat_col_pins[x]);
	}

	for (y = 0; y < KEY_ROWS_HALF; y++) {
		gpio_init(keymat_row_pins[y]);
		gpio_set_dir(keymat_row_pins[y], GPIO_OUT);
	}
}

void
keymat_next(void)
{
	memcpy(keymat_prev, keymat, sizeof(keymat));
}

void
keymat_scan(void)
{
	bool (*keymat_half)[KEY_COLS];
	uint x, y;

	keymat_half = KEYMAT_HALF(SPLIT_SIDE);
	for (y = 0; y < KEY_ROWS_HALF; y++) {
		gpio_put(keymat_row_pins[y], 0);
		busy_wait_us(5);
		for (x = 0; x < KEY_COLS; x++)
			keymat_half[y][x] = !gpio_get(keymat_col_pins[x]);
		gpio_put(keymat_row_pins[y], 1);
		busy_wait_us(5);
	}
}

uint32_t
keymat_encode_half(int side)
{
	bool (*keymat_half)[KEY_COLS];
	uint32_t mask;
	uint x, y;

	mask = 0;
	keymat_half = KEYMAT_HALF(side);
	for (y = 0; y < KEY_ROWS_HALF; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			if (keymat_half[y][x])
				mask |= 1 << (y * KEY_COLS + x);
		}
	}

	return mask;
}

void
keymat_decode_half(int side, uint32_t mask)
{
	bool (*keymat_half)[KEY_COLS];
	uint x, y;

	keymat_half = KEYMAT_HALF(side);
	for (y = 0; y < KEY_ROWS_HALF; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			keymat_half[y][x] = (mask >> (y * KEY_COLS + x)) & 1;
		}
	}
}
