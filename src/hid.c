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

struct hid_report {
	uint8_t mods;
	uint8_t codes[6];
	uint8_t codecnt;
};

static uint32_t keysyms[KEY_ROWS][KEY_COLS] = { 0 };

static struct layerkey active_stack[16] = { 0 };
static uint active_top = 0;

static struct hid_report hid_report_prev;
static struct hid_report hid_report;

static uint8_t active_weak_mods;
static uint8_t active_mods;

static uint64_t bounce_mat[KEY_ROWS][KEY_COLS] = { 0 };

static bool seen_mat[KEY_ROWS][KEY_COLS];

static void active_pop(uint layer);
static void active_push(uint layer, uint key);
static void add_keycode(uint8_t keycode);

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
	uint i;

	if (active_top == ARRLEN(active_stack) - 1) {
		WARN("Active stack overflow");
		return;
	}

	active_top += 1;
	active_stack[active_top].layer = layer;
	active_stack[active_top].key = key;

	for (i = 0; i <= active_top; i++) {
		INFO("%i. ACTIVE %u %u", i,
			active_stack[i].layer, active_stack[i].key);
	}
}

void
add_keycode(uint8_t keycode)
{
	if (hid_report.codecnt >= 6) {
		WARN("HID report overflow");
		return;
	}

	hid_report.codes[hid_report.codecnt] = keycode;
	hid_report.codecnt++;
}

uint8_t
parse_modifiers(uint32_t keysym)
{
	uint8_t mods;

	mods = 0;

	if (IS_LEFT_CTRL(keysym))
		mods |= MOD_BIT(KC_LEFT_CTRL);

	if (IS_RIGHT_CTRL(keysym))
		mods |= MOD_BIT(KC_RIGHT_CTRL);

	if (IS_LEFT_SHIFT(keysym))
		mods |= MOD_BIT(KC_LEFT_SHIFT);

	if (IS_RIGHT_SHIFT(keysym))
		mods |= MOD_BIT(KC_RIGHT_SHIFT);

	if (IS_LEFT_GUI(keysym))
		mods |= MOD_BIT(KC_LEFT_GUI);

	if (IS_RIGHT_GUI(keysym))
		mods |= MOD_BIT(KC_RIGHT_GUI);

	if (IS_LEFT_ALT(keysym))
		mods |= MOD_BIT(KC_LEFT_ALT);

	if (IS_RIGHT_ALT(keysym))
		mods |= MOD_BIT(KC_RIGHT_ALT);

	return mods;
}

void
handle_keypress_new(uint x, uint y)
{
	uint32_t ksym;
	int i;

	for (i = (int) active_top; i >= 0; i--) {
		ksym = keymap_layers[active_stack[i].layer][y][x];
		if (ksym == KC_NO) return;
		if (ksym != KC_TRNS)
			break;
	}
	if (i < 0) return;
	keysyms[y][x] = ksym;

	if (IS_SWITCH(keysyms[y][x])) {
		active_push(TO_LAYER(keysyms[y][x]), y * KEY_COLS + x);
	} else if (IS_USER(keysyms[y][x])) {
		process_user_keypress_new(TO_SYM(keysyms[y][x]), x, y);
	} else if (IS_KC(keysyms[y][x]) && IS_KEY_KC(TO_KC(keysyms[y][x]))) {
		/* FIXME: two keys pressed at the exact same time with
		 * different weak modifiers will not be reported correctly */
		active_weak_mods = parse_modifiers(keysyms[y][x]);
	}
}

void
handle_keypress(uint x, uint y)
{
	if (!keymat_prev[y][x])
		handle_keypress_new(x, y);

	if (seen_mat[y][x]) return;
	seen_mat[y][x] = true;

	if (IS_KC(keysyms[y][x]) && IS_KEY_KC(TO_KC(keysyms[y][x]))) {
		add_keycode(TO_KC(keysyms[y][x]));
		INFO("CODE %u %u", active_top, keysyms[y][x]);
	} else if (IS_KC(keysyms[y][x]) && IS_MOD_KC(TO_KC(keysyms[y][x]))) {
		active_mods |= MOD_BIT(TO_KC(keysyms[y][x]));
	} else if (IS_MOD(keysyms[y][x])) {
		active_mods |= parse_modifiers(keysyms[y][x]);
	}
}

void
handle_keyrelease_new(uint x, uint y)
{
	uint i;

	if (IS_USER(keysyms[y][x]))
		process_user_keyrelease_new(TO_SYM(keysyms[y][x]), x, y);

	for (i = 1; i <= active_top; i++) {
		if (active_stack[i].key == y * KEY_COLS + x) {
			active_pop(i);
			break;
		}
	}
}

void
handle_keyrelease(uint x, uint y)
{
	if (keymat_prev[y][x])
		handle_keyrelease_new(x, y);
}

void
update_report(void)
{
	uint64_t now_us;
	uint x, y;

	now_us = time_us_64();
	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			if (keymat[y][x] != keymat_prev[y][x]) {
				if (bounce_mat[y][x] > now_us - 50000) {
					DEBUG("Bouncing prevented %i vs %i",
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
}

bool
send_keyboard_report(void)
{
	hid_report.mods = active_weak_mods | active_mods;
	if (memcmp(&hid_report, &hid_report_prev, sizeof(hid_report))) {
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD,
			hid_report.mods, hid_report.codes);
		memcpy(&hid_report_prev, &hid_report, sizeof(hid_report));
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
hid_init(void)
{
}

void
hid_force_release(uint x, uint y)
{
	handle_keyrelease_new(x, y);
	keysyms[y][x] = KC_NO;
}

void
hid_switch_layer_with_key(uint8_t layer, uint x, uint y)
{
	active_push(layer, y * KEY_COLS + x);
	keysyms[y][x] = SW(layer);
	seen_mat[y][x] = true;
}

void
hid_task(void)
{
	update_report();
	if (tud_hid_ready()) {
		send_hid_report(REPORT_ID_MIN);
		memset(&hid_report, 0, sizeof(hid_report));
		memset(seen_mat, 0, sizeof(seen_mat));
		active_mods = 0;
	}
}
