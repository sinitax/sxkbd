#include "hid.h"
#include "keycode.h"
#include "split.h"
#include "keymat.h"
#include "keysym.h"
#include "keymap.h"

#include "hardware/timer.h"
#include "bsp/board.h"
#include "pico/types.h"

#include <string.h>

struct layerkey {
	uint layer;
	uint key;
};

static uint32_t keysyms[KEY_ROWS][KEY_COLS] = { 0 };

static struct layerkey active_stack[16] = { 0 };
static uint active_top = 0;

static uint8_t hid_report_prev[6] = { 0 };
static uint8_t hid_report[6] = { 0 };
static uint hid_report_len = 0;

static uint64_t bounce_mat[KEY_ROWS][KEY_COLS] = { 0 };

static bool seen_mat[KEY_ROWS][KEY_COLS];

static void active_pop(uint layer);
static void active_push(uint layer, uint key);

void
active_pop(uint layer)
{
	uint i;

	for (i = layer + 1; i <= active_top; i++)
		active_stack[i-1] = active_stack[i];
	if (layer <= active_top)
		active_top--;
}

void
active_push(uint layer, uint key)
{
	if (active_top == ARRLEN(active_stack) - 1) {
		WARN("Active stack overflow");
		return;
	}
	active_top += 1;
	active_stack[active_top].layer = layer;
	active_stack[active_top].key = key;
}

void
add_keycode(uint8_t keycode)
{
	if (hid_report_len >= 6) {
		WARN("HID report overflow");
		return;
	}

	hid_report[hid_report_len] = keycode;
	hid_report_len++;
}

void
handle_keypress(uint x, uint y)
{
	uint32_t ksym;
	int i;

	if (!keymat_prev[y][x]) {
		for (i = (int) active_top; i >= 0; i--) {
			ksym = keymap_layers[active_stack[i].layer][y][x];
			if (ksym == KC_NO) return;
			if (ksym != KC_TRNS)
				break;
		}
		if (i < 0) return;
		keysyms[y][x] = ksym;

		if (IS_SWITCH(keysyms[y][x])) {
			INFO("LAYER %u", TO_LAYER(keysyms[y][x]));
			active_push(TO_LAYER(ksym), y * KEY_COLS + x);
			for (i = 0; i <= (int) active_top; i++) {
				INFO("%i. ACTIVE %u %u", i,
					active_stack[i].layer, active_stack[i].key);
			}
		}
	}

	if (!seen_mat[y][x]) {
		if (IS_CTRL(keysyms[y][x])) {
			if (IS_RIGHT(keysyms[y][x])) {
				add_keycode(KC_RIGHT_CTRL);
			} else {
				add_keycode(KC_LEFT_CTRL);
			}
		}

		if (IS_SHIFT(keysyms[y][x])) {
			if (IS_RIGHT(keysyms[y][x])) {
				add_keycode(KC_RIGHT_SHIFT);
			} else {
				add_keycode(KC_LEFT_SHIFT);
			}
		}

		if (IS_ALT(keysyms[y][x])) {
			if (IS_RIGHT(keysyms[y][x])) {
				add_keycode(KC_RIGHT_ALT);
			} else {
				add_keycode(KC_LEFT_ALT);
			}
		}

		if (IS_GUI(keysyms[y][x])) {
			if (IS_RIGHT(keysyms[y][x])) {
				add_keycode(KC_RIGHT_GUI);
			} else {
				add_keycode(KC_LEFT_GUI);
			}
		}

		if (IS_CODE(keysyms[y][x])) {
			add_keycode(TO_CODE(keysyms[y][x]));
			INFO("CODE %u %u", active_top, keysyms[y][x]);
		}

		seen_mat[y][x] = true;
	}
}

void
handle_keyrelease(uint x, uint y)
{
	uint i;

	if (keymat_prev[y][x]) {
		for (i = 1; i <= active_top; i++) {
			if (active_stack[i].key == y * KEY_COLS + x) {
				active_pop(i);
				break;
			}
		}
	}
}

bool
update_report(void)
{
	uint64_t now_us;
	uint keycnt;
	uint x, y;

	keycnt = 0;
	now_us = time_us_64();
	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			if (keymat[y][x] != keymat_prev[y][x]) {
				if (bounce_mat[y][x] > now_us - 50000) {
					WARN("Bouncing prevented %i vs %i",
						keymat[y][x], keymat_prev[y][x]);
					keymat[y][x] = keymat_prev[y][x];
				} else {
					bounce_mat[y][x] = now_us;
				}
			}

			if (keymat[y][x]) {
				handle_keypress(x, y);
			} else {
				handle_keyrelease(x, y);
			}
		}
	}

	return keycnt > 0;
}

void
hid_init(void)
{
}

bool
send_keyboard_report(void)
{
	uint i;
	if (memcmp(hid_report, hid_report_prev, sizeof(hid_report))) {
		for (i = 0; i < 6; i++)
			INFO("REPORT %u: %u", i, hid_report[i]);
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, hid_report);
		memcpy(hid_report_prev, hid_report, sizeof(hid_report));
		return true;
	}

	return false;
}

bool
send_mouse_report(bool state)
{
	if (state) {
		tud_hid_mouse_report(REPORT_ID_MOUSE, 0, 10, 10, 0, 0);
		return true;
	}

	return false;
}

bool
send_consumer_control_report(bool state)
{
	static bool cleared = true;
	uint16_t report;

	if (state) {
		report = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
		tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report, 2);
		cleared = false;
		return true;
	} else if (!cleared) {
		report = 0;
		tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report, 2);
		cleared = true;
		return true;
	}

	return false;
}

bool
send_hid_report(int id)
{
	switch (id) {
	case REPORT_ID_KEYBOARD:
		return send_keyboard_report();
	case REPORT_ID_MOUSE:
		return send_mouse_report(false);
	case REPORT_ID_CONSUMER_CONTROL:
		return send_consumer_control_report(false);
	}

	return false;
}

void
tud_hid_report_complete_cb(uint8_t instance,
	uint8_t const *report, uint8_t len)
{
	uint8_t id;

	for (id = report[0] + 1; id < REPORT_ID_MAX; id++) {
		if (send_hid_report(id))
			break;
	}
}

void
hid_task(void)
{
	update_report();
	if (tud_hid_ready()) {
		send_hid_report(REPORT_ID_MIN);
		memset(hid_report, 0, sizeof(hid_report));
		memset(seen_mat, 0, sizeof(seen_mat));
		hid_report_len = 0;
	}
}
