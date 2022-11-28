#include "board.h"
#include "util.h"

#include "hardware/timer.h"

#define KEY_ROWS ARRLEN(matrix_row_pins)
#define KEY_COLS ARRLEN(matrix_col_pins)
#define KEY_COUNT (KEY_ROWS * KEY_COLS)

static void handle_press(uint x, uint y);
static void handle_release(uint x, uint y);

static const uint matrix_col_pins[] = { 4, 5, 6, 7 };
static const uint matrix_row_pins[] = { 29, 28, 27, 26, 22, 20 };

//static uint press_index = 0;
static bool press_map[KEY_COUNT] = { 0 };
//static uint32_t press_syms[KEY_COUNT] = { 0 };
//static uint8_t press_order[KEY_COUNT] = { 0 };

void
handle_press(uint x, uint y)
{
	
}

void
handle_release(uint x, uint y)
{

}

void
hid_init(void)
{
	uint x, y;

	for (y = 0; y < ARRLEN(matrix_row_pins); y++) {
		gpio_init(matrix_row_pins[y]);
		gpio_set_dir(matrix_row_pins[y], GPIO_IN);
		gpio_pull_up(matrix_row_pins[y]);
	}

	for (x = 0; x < ARRLEN(matrix_col_pins); x++) {
		gpio_init(matrix_col_pins[x]);
		gpio_set_dir(matrix_col_pins[x], GPIO_OUT);
	}
}

void
hid_task(void)
{
	bool pressed;
	uint x, y;

	/* scan matrix */
	for (y = 0; y < ARRLEN(matrix_row_pins); y++) {
		for (x = 0; x < ARRLEN(matrix_col_pins); x++) {
			gpio_put(matrix_col_pins[x], 0);
			busy_wait_us(5);
			pressed = !gpio_get(matrix_row_pins[y]);
			if (pressed && !press_map[y * KEY_COLS + x]) {
				handle_press(x, y);
				press_map[y * KEY_COLS + x] = true;
			} else if (!pressed && press_map[y * KEY_COLS + x]) {
				handle_release(x, y);
				press_map[y * KEY_COLS + x] = false;
			}
			gpio_put(matrix_col_pins[x], 1);
			busy_wait_us(5);
		}
	}

	/* assemble hid report */
}
