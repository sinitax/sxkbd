#include "keymat.h"

#include "keymap.h"
#include "split.h"
#include "util.h"

#include "bsp/board.h"
#include "pico/types.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include <string.h>

#define DEBOUNCE_MS 50

#ifdef BAD_GPIO_MITIGATION
static const uint keymat_row_pins[] = { 4, 9, 6, 7 };
#else
static const uint keymat_row_pins[] = { 4, 5, 6, 7 };
#endif

static const uint keymat_col_pins[] = { 29, 28, 27, 26, 22, 20 };
static_assert(ARRLEN(keymat_row_pins) == KEY_ROWS_HALF);
static_assert(ARRLEN(keymat_col_pins) == KEY_COLS);

bool keymat_prev[KEY_ROWS][KEY_COLS];
bool keymat[KEY_ROWS][KEY_COLS];

static uint32_t keymat_modt[KEY_ROWS_HALF][KEY_COLS];

void
keymat_init(void)
{
	uint x, y;

	for (x = 0; x < KEY_COLS; x++) {
		gpio_init(keymat_col_pins[x]);
		gpio_set_dir(keymat_col_pins[x], GPIO_IN);
		gpio_set_drive_strength(keymat_col_pins[x], GPIO_DRIVE_STRENGTH_2MA);
		gpio_set_slew_rate(keymat_col_pins[x], GPIO_SLEW_RATE_FAST);
		gpio_pull_up(keymat_col_pins[x]);
	}

	for (y = 0; y < KEY_ROWS_HALF; y++) {
		gpio_init(keymat_row_pins[y]);
		gpio_set_dir(keymat_row_pins[y], GPIO_OUT);
		gpio_set_drive_strength(keymat_col_pins[x], GPIO_DRIVE_STRENGTH_2MA);
		gpio_set_slew_rate(keymat_row_pins[y], GPIO_SLEW_RATE_FAST);
		gpio_put(keymat_row_pins[y], 0);
	}
}

bool
keymat_scan(void)
{
	bool (*keymat_half)[KEY_COLS];
	bool state, modified;
	uint32_t now_ms;
	uint x, y;

	now_ms = board_millis();
	modified = false;

	memcpy(keymat_prev, keymat, sizeof(keymat));
	keymat_half = KEYMAT_HALF(keymat, SPLIT_SIDE);
	for (y = 0; y < KEY_ROWS_HALF; y++) {
		gpio_put(keymat_row_pins[y], 0);
		busy_wait_us(5);
		for (x = 0; x < KEY_COLS; x++) {
			if (keymat_modt[y][x] > now_ms - DEBOUNCE_MS)
				continue;
			state = !gpio_get(keymat_col_pins[x]);
			if (state != keymat_half[y][x]) {
				modified = true;
				keymat_half[y][x] = state;
				keymat_modt[y][x] = now_ms;
			}
		}
		gpio_put(keymat_row_pins[y], 1);
		busy_wait_us(5);
	}

	return modified;
}

uint32_t
keymat_encode_half(int side)
{
	bool (*keymat_half)[KEY_COLS];
	uint32_t mask;
	uint x, y;

	mask = 0;
	keymat_half = KEYMAT_HALF(keymat, side);
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

	keymat_half = KEYMAT_HALF(keymat, side);
	for (y = 0; y < KEY_ROWS_HALF; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			keymat_half[y][x] = (mask >> (y * KEY_COLS + x)) & 1;
		}
	}
}

void
keymat_debug(void)
{
	uint x, y;

	if (log_level_min > LOG_DEBUG)
		return;

	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			if (!keymat_prev[y][x] && keymat[y][x])
				DEBUG(LOG_KEYMAT, "Key pressed: %u %u", x, y);
			else if (keymat_prev[y][x] && !keymat[y][x])
				DEBUG(LOG_KEYMAT, "Key released: %u %u", x, y);
		}
	}
}
