#include "hid.h"
#include "split.h"
#include "matrix.h"
#include "keysym.h"
#include "keymap.h"

#include <pico/types.h>

void
hid_init(void)
{
	
}

inline uint
layer_index(uint x, uint y)
{
	if (y < KEY_ROWS)
		return y * KEY_COLS + x;
	else
		return y * KEY_COLS + (KEY_COLS - 1 - x);
}

bool
hid_gen_report(uint8_t *report)
{
	int keycnt;
	uint x, y, p;

	keycnt = 0;
	for (y = 0; y < KEY_ROWS * 2; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			if (!state_matrix[y * KEY_COLS + x])
				continue;
			if (keycnt >= 6) break;
			DEBUG("PRESS %i %", x, y);
			p = layer_index(x, y);
			report[keycnt] = TO_CODE(keymap_layers_de[0][p]);
			keycnt++;
		}
	}

	return keycnt > 0;
}

void
hid_task(void)
{

	/* assemble hid report */
}
