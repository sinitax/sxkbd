#include "hid.h"

#include "keysym/consumer.h"
#include "keysym/system.h"
#include "hid/keyboard.h"
#include "hid/consumer.h"
#include "hid/system.h"

#include "split.h"
#include "keymat.h"
#include "keysym.h"
#include "keymap.h"

#include "hardware/timer.h"
#include "bsp/board.h"
#include "pico/types.h"

#include <stdbool.h>
#include <string.h>

struct layerkey {
	uint layer;
	uint key;
};

struct hid_keyboard_report {
	uint8_t mods;
	uint8_t codes[6];
	uint8_t cnt;
};

struct hid_mouse_report {
	uint8_t btns;
	int8_t x, y;
	int8_t v, h;
};

struct hid_consumer_report {
	uint16_t code;
};

struct hid_system_report {
	uint16_t code;
};

struct hid_gamepad_report {
	int8_t x, y, z;
	int8_t rx, ry, rz;
	uint8_t hat;
	uint32_t btns;
};

static uint32_t keysyms[KEY_ROWS][KEY_COLS] = { 0 };

static struct layerkey active_stack[16] = { 0 };
static uint active_top = 0;

static struct hid_keyboard_report keyboard_report_prev = { 0 };
static struct hid_keyboard_report keyboard_report = { 0 };

static struct hid_mouse_report mouse_report_prev = { 0 };
static struct hid_mouse_report mouse_report = { 0 };

static struct hid_consumer_report consumer_report_prev = { 0 };
static struct hid_consumer_report consumer_report = { 0 };

static struct hid_system_report system_report_prev = { 0 };
static struct hid_system_report system_report = { 0 };

static struct hid_gamepad_report gamepad_report_prev = { 0 };
static struct hid_gamepad_report gamepad_report = { 0 };

static uint8_t active_weak_mods = 0;
static uint8_t active_mods = 0;

static uint64_t bounce_mat[KEY_ROWS][KEY_COLS] = { 0 };

static bool seen_mat[KEY_ROWS][KEY_COLS] = { 0 };

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
	if (keyboard_report.cnt >= 6) {
		WARN("HID report overflow");
		return;
	}

	keyboard_report.codes[keyboard_report.cnt] = keycode;
	keyboard_report.cnt++;
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
	} else if (IS_CONSUMER(keysyms[y][x])) {
		consumer_report.code = keysym_to_consumer(keysyms[y][x]);
		INFO("CONSUMER KEY %i", consumer_report.code);
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
	bool sent;

	sent = false;
	keyboard_report.mods = active_weak_mods | active_mods;
	if (memcmp(&keyboard_report, &keyboard_report_prev,
			sizeof(keyboard_report))) {
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD,
			keyboard_report.mods, keyboard_report.codes);
		memcpy(&keyboard_report_prev, &keyboard_report,
			sizeof(keyboard_report));
		sent = true;
	}

	memset(&keyboard_report, 0, sizeof(keyboard_report));
	active_mods = 0;
	memset(seen_mat, 0, sizeof(seen_mat));

	return sent;
}

bool
send_consumer_report(void)
{
	bool sent;

	INFO("CONSUMER %u", consumer_report.code);

	sent = false;
	if (memcmp(&consumer_report, &consumer_report_prev,
			sizeof(consumer_report))) {
		tud_hid_report(REPORT_ID_CONSUMER,
			&consumer_report.code, 2);
		memcpy(&consumer_report_prev, &consumer_report,
			sizeof(consumer_report));
		return true;
	}

	memset(&consumer_report, 0, sizeof(consumer_report));

	return sent;
}

bool
send_system_report(void)
{
	bool sent;

	sent = false;
	if (memcmp(&system_report, &system_report_prev,
			sizeof(system_report))) {
		tud_hid_report(REPORT_ID_SYSTEM,
			&system_report.code, 2);
		memcpy(&system_report_prev, &system_report,
			sizeof(system_report));
		sent = true;
	}

	memset(&system_report, 0, sizeof(system_report));

	return sent;
}

bool
send_mouse_report(void)
{
	bool sent;

	sent = false;
	if (memcmp(&mouse_report, &mouse_report_prev,
			sizeof(mouse_report))) {
		tud_hid_mouse_report(REPORT_ID_KEYBOARD,
			mouse_report.btns,
			mouse_report.x, mouse_report.y,
			mouse_report.h, mouse_report.v);
		memcpy(&mouse_report_prev, &mouse_report,
			sizeof(mouse_report));
		sent = true;
	}

	memset(&mouse_report, 0, sizeof(mouse_report));

	return sent;
}

bool
send_gamepad_report(void)
{
	bool sent;

	sent = false;
	if (memcmp(&gamepad_report, &gamepad_report_prev,
			sizeof(gamepad_report))) {
		tud_hid_gamepad_report(REPORT_ID_GAMEPAD,
			gamepad_report.x, gamepad_report.y, gamepad_report.z,
			gamepad_report.rz, gamepad_report.rx, gamepad_report.ry,
			gamepad_report.hat, gamepad_report.btns);
		memcpy(&gamepad_report_prev, &gamepad_report,
			sizeof(gamepad_report));
		sent = true;
	}

	memset(&gamepad_report, 0, sizeof(gamepad_report));

	return sent;
}

bool
send_hid_report(int id)
{
	switch (id) {
	case REPORT_ID_KEYBOARD:
		return send_keyboard_report();
	case REPORT_ID_CONSUMER:
		return send_consumer_report();
	case REPORT_ID_SYSTEM:
		return send_system_report();
	case REPORT_ID_MOUSE:
		return send_mouse_report();
	case REPORT_ID_GAMEPAD:
		return send_gamepad_report();
	}

	return false;
}

void
send_next_hid_report(uint8_t min)
{
	uint8_t id;

	for (id = min; id < REPORT_ID_MAX; id++) {
		if (send_hid_report(id))
			break;
	}
}

void
tud_hid_report_complete_cb(uint8_t instance,
	uint8_t const *report, uint8_t len)
{
	send_next_hid_report(report[0] + 1);
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
	if (tud_hid_ready())
		send_next_hid_report(REPORT_ID_MIN);
}
