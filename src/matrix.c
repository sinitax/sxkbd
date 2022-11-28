#include "keymap.h"

#include "matrix.h"
#include "pico/types.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include <string.h>

static const uint matrix_row_pins[] = { 4, 5, 6, 7 };
static const uint matrix_col_pins[] = { 29, 28, 27, 26, 22, 20 };
static_assert(ARRLEN(matrix_row_pins) == KEY_ROWS);
static_assert(ARRLEN(matrix_col_pins) == KEY_COLS);

bool prev_state_matrix[KEY_COUNT];
bool state_matrix[KEY_COUNT];

void
matrix_init(void)
{
	uint x, y;

	for (y = 0; y < KEY_ROWS; y++) {
		gpio_init(matrix_row_pins[y]);
		gpio_set_dir(matrix_row_pins[y], GPIO_IN);
		gpio_pull_up(matrix_row_pins[y]);
	}

	for (x = 0; x < KEY_COLS; x++) {
		gpio_init(matrix_col_pins[x]);
		gpio_set_dir(matrix_col_pins[x], GPIO_OUT);
	}
}

void
scan_matrix(void)
{
	bool pressed;
	uint x, y;

	memcpy(prev_state_matrix, state_matrix, sizeof(state_matrix));

	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			gpio_put(matrix_col_pins[x], 0);
			busy_wait_us(5);
			pressed = !gpio_get(matrix_row_pins[y]);
			state_matrix[y * KEY_COLS + x] = pressed;
			gpio_put(matrix_col_pins[x], 1);
			busy_wait_us(5);
		}
	}
}
