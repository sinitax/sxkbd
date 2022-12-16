#include "keymap.h"

#include "matrix.h"
#include "split.h"
#include "pico/types.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include <string.h>

static const uint matrix_row_pins[] = { 4, 5, 6, 7 };
static const uint matrix_col_pins[] = { 29, 28, 27, 26, 22, 20 };
static_assert(ARRLEN(matrix_row_pins) == KEY_ROWS);
static_assert(ARRLEN(matrix_col_pins) == KEY_COLS);

bool prev_state_matrix[KEY_COUNT * 2];
bool state_matrix[KEY_COUNT * 2];

void
matrix_init(void)
{
	uint x, y;

	for (x = 0; x < KEY_COLS; x++) {
		gpio_init(matrix_col_pins[x]);
		gpio_set_dir(matrix_col_pins[x], GPIO_IN);
		gpio_pull_up(matrix_col_pins[x]);
	}

	for (y = 0; y < KEY_ROWS; y++) {
		gpio_init(matrix_row_pins[y]);
		gpio_set_dir(matrix_row_pins[y], GPIO_OUT);
	}
}

void
scan_matrix(void)
{
	bool pressed;
	uint x, y, p;

	memcpy(prev_state_matrix, state_matrix, sizeof(state_matrix));

	for (y = 0; y < KEY_ROWS; y++) {
		gpio_put(matrix_row_pins[y], 0);
		busy_wait_us(5);
		for (x = 0; x < KEY_COLS; x++) {
			pressed = !gpio_get(matrix_col_pins[x]);
			p = MAT_OFFSET(SPLIT_SIDE) + y * KEY_COLS + x;
			state_matrix[p] = pressed;
		}
		gpio_put(matrix_row_pins[y], 1);
		busy_wait_us(5);
	}
}

uint32_t
matrix_encode_half(int side)
{
	uint32_t mask;
	uint x, y, p;

	mask = 0;
	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			p = MAT_OFFSET(side) + y * KEY_COLS + x;
			if (state_matrix[p])
				mask |= 1 << (y * KEY_COLS + x);
		}
	}

	return mask;
}

void
matrix_decode_half(int side, uint32_t mask)
{
	uint x, y, p;

	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			p = MAT_OFFSET(side) + y * KEY_COLS + x;
			state_matrix[p] = (mask >> (y * KEY_COLS + x)) & 1;
		}
	}
}
