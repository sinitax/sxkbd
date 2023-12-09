#include "hid.h"

#include "led.h"
#include "split.h"
#include "keymat.h"
#include "keysym.h"
#include "keymap.h"

#include "class/hid/hid.h"
#include "keysym/consumer.h"
#include "keysym/system.h"
#include "hid/keyboard.h"
#include "hid/consumer.h"
#include "hid/system.h"
#include "hardware/timer.h"
#include "bsp/board.h"
#include "pico/types.h"

#include <stdbool.h>
#include <string.h>

#define HID_REPORT_CODES 6

#define MACRO_X 0
#define MACRO_Y 7

struct layerkey {
	uint layer;
	uint key;
};

struct hid_keyboard_report {
	uint8_t mods, weak_mods;
	uint8_t codes[HID_REPORT_CODES];
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

static struct layerkey active_layers[16] = { 0 };
static uint active_layers_top = 0;

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

static uint32_t macro_held_stack[MACRO_HOLD_MAX] = { 0 };
static uint macro_held_cnt = 0;

static bool macro_running = false;

/* TODO replace these two with stack method primitives (added to util) */

static void active_layers_reset(void);
static void active_layers_pop(uint layer);
static void active_layers_push(uint layer, uint key);

static void macro_held_reset(void);
static void macro_held_pop(uint32_t keysym);
static void macro_held_push(uint32_t keysym);
static bool macro_held_find(uint32_t keysym);

static void add_keycode(uint8_t keycode);

/* TODO: add static prototypes */

void
active_layers_reset(void)
{
	active_layers_top = 0;
}

bool
active_layers_find(uint layer)
{
	uint i;

	for (i = 0; i <= active_layers_top; i++) {
		if (active_layers[i].layer == layer)
			return true;
	}

	return false;
}

void
active_layers_pop(uint layer)
{
	uint i;

	for (i = layer + 1; i <= active_layers_top; i++)
		active_layers[i-1] = active_layers[i];
	if (layer <= active_layers_top)
		active_layers_top--;
}

void
active_layers_push(uint layer, uint key)
{
	uint i;

	if (active_layers_top == ARRLEN(active_layers) - 1) {
		WARN(LOG_KEYMAP, "Active stack overflow");
		return;
	}

	active_layers_top += 1;
	active_layers[active_layers_top].layer = layer;
	active_layers[active_layers_top].key = key;

	for (i = 0; i <= active_layers_top; i++) {
		DEBUG(LOG_KEYMAP, "%i. ACTIVE %u %u", i,
			active_layers[i].layer, active_layers[i].key);
	}
}

void
macro_held_reset(void)
{
	macro_held_cnt = 0;
}

void
macro_held_pop(uint32_t keysym)
{
	uint i, cnt;

	for (i = cnt = 0; i < macro_held_cnt; i++) {
		if (macro_held_stack[i] != keysym) {
			macro_held_stack[cnt] = macro_held_stack[i];
			cnt++;
		}
	}
	macro_held_cnt = cnt;
}

void
macro_held_push(uint32_t keysym)
{
	if (macro_held_cnt == MACRO_HOLD_MAX) {
		WARN(LOG_KEYMAP, "Macro held keys overflow");
		return;
	}

	macro_held_stack[macro_held_cnt] = keysym;
	macro_held_cnt++;
}

bool
macro_held_find(uint32_t keysym)
{
	uint i;

	for (i = 0; i < macro_held_cnt; i++) {
		if (macro_held_stack[i] == keysym)
			return true;
	}

	return false;
}

void
add_keycode(uint8_t keycode)
{
	if (keyboard_report.cnt >= 6) {
		WARN(LOG_HID, "HID report overflow");
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

uint32_t
determine_keysym(uint x, uint y)
{
	uint32_t keysym;
	int i;

	keysym = KC_NO;
	for (i = (int) active_layers_top; i >= 0; i--) {
		keysym = keymap_layers[active_layers[i].layer][y][x];
		if (keysym != KC_TRNS && keysym != KC_NO)
			break;
	}

	return keysym;
}

void
process_keypress(uint32_t keysym, uint x, uint y)
{
	if (IS_SWITCH(keysym)) {
		active_layers_push(TO_LAYER(keysym), y * KEY_COLS + x);
	} else if (IS_TOGGLE(keysym)) {
		if (active_layers_find(TO_LAYER(keysym)))
			active_layers_pop(TO_LAYER(keysym));
		else
			active_layers_push(TO_LAYER(keysym), y * KEY_COLS + x);
	} else if (IS_REBASE(keysym)) {
		active_layers_reset();
		active_layers[0].layer = TO_LAYER(keysym);
	} else if (IS_USER(keysym)) {
		process_user_keypress(TO_USER(keysym), x, y);
	} else if (IS_KC(keysym) && IS_KEY_KC(TO_KC(keysym))) {
		/* FIXME: two keys pressed at the exact same time with
		 * different weak modifiers will not be reported correctly */
		active_weak_mods = parse_modifiers(keysym);
	} else if (IS_CONSUMER(keysym)) {
		consumer_report.code = keysym_to_consumer(keysym);
	}
}

void
process_keydown(uint32_t keysym, uint x, uint y)
{
	if (x != MACRO_X || y != MACRO_Y) {
		if (seen_mat[y][x]) return;
		seen_mat[y][x] = true;
	}

	if (IS_KC(keysym) && IS_KEY_KC(TO_KC(keysym))) {
		add_keycode(TO_KC(keysym));
	} else if (IS_KC(keysym) && IS_MOD_KC(TO_KC(keysym))) {
		active_mods |= MOD_BIT(TO_KC(keysym));
	} else if (IS_SWITCH(keysym) && IS_MOD(keysym)) {
		active_mods |= parse_modifiers(keysym);
	}
}

void
process_keyrelease(uint32_t keysym, uint x, uint y)
{
	uint i;

	if (IS_USER(keysym))
		process_user_keyrelease(TO_USER(keysym), x, y);

	for (i = 1; i <= active_layers_top; i++) {
		if (active_layers[i].key == y * KEY_COLS + x) {
			active_layers_pop(i);
			break;
		}
	}
}

void
process_keyup(uint32_t keysym, uint x, uint y)
{
}

void
process_key(uint x, uint y, uint64_t now_us)
{
	if (keymat[y][x] != keymat_prev[y][x]) {
		if (bounce_mat[y][x] > now_us - 50000) {
			DEBUG(LOG_KEYMAT, "Bouncing prevented %i vs %i",
				keymat[y][x], keymat_prev[y][x]);
			keymat[y][x] = keymat_prev[y][x];
		} else {
			bounce_mat[y][x] = now_us;
		}
	}

	if (keymat[y][x] && !keymat_prev[y][x])
		keysyms[y][x] = determine_keysym(x, y);

	if (keymat[y][x]) {
		if (!keymat_prev[y][x])
			process_keypress(keysyms[y][x], x, y);
		process_keydown(keysyms[y][x], x, y);
	} else {
		if (keymat_prev[y][x])
			process_keyrelease(keysyms[y][x], x, y);
		process_keyup(keysyms[y][x], x, y);
	}
}

void
update_report(void)
{
	uint64_t now_us;
	uint x, y;

	now_us = time_us_64();
	for (y = 0; y < KEY_ROWS; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			process_key(x, y, now_us);
		}
	}
}

bool
update_keyboard_report(struct hid_keyboard_report *new,
	struct hid_keyboard_report *old)
{
	return new->mods != old->mods
		|| memcmp(new->codes, old->codes, HID_REPORT_CODES);
}

bool
update_weak_mods(struct hid_keyboard_report *new,
	struct hid_keyboard_report *old)
{
	int i, k;

	/* only need weak modes if new keycode added */
	for (i = 0; i < new->cnt; i++) {
		for (k = 0; k < old->cnt; k++) {
			if (new->codes[i] != old->codes[k])
				break;
		}
		if (k == old->cnt)
			return true;
	}

	return false;
}

bool
send_keyboard_report(void)
{
	bool sent;

	sent = false;

	keyboard_report.mods = active_mods;
	if (update_weak_mods(&keyboard_report, &keyboard_report_prev))
		keyboard_report.weak_mods = active_weak_mods;

	if (update_keyboard_report(&keyboard_report, &keyboard_report_prev)) {
		tud_hid_n_keyboard_report(INST_HID_KBD, REPORT_ID_NONE,
			keyboard_report.mods | keyboard_report.weak_mods,
			keyboard_report.codes);
		memcpy(&keyboard_report_prev, &keyboard_report,
			sizeof(keyboard_report));
		sent = true;

		active_weak_mods = 0;
	}

	active_mods = 0;

	memset(keyboard_report.codes, 0, HID_REPORT_CODES);
	keyboard_report.cnt = 0;

	memset(seen_mat, 0, sizeof(seen_mat));

	return sent;
}

bool
send_mouse_report(void)
{
	bool sent;

	sent = false;
	if (memcmp(&mouse_report, &mouse_report_prev,
			sizeof(mouse_report))) {
		tud_hid_n_mouse_report(INST_HID_MISC,
			REPORT_ID_MOUSE, mouse_report.btns,
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
send_consumer_report(void)
{
	bool sent;

	sent = false;
	if (memcmp(&consumer_report, &consumer_report_prev,
			sizeof(consumer_report))) {
		INFO(LOG_HID, "CONSUMER SEND");
		tud_hid_n_report(INST_HID_MISC, REPORT_ID_CONSUMER,
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
		tud_hid_n_report(INST_HID_MISC, REPORT_ID_SYSTEM,
			&system_report.code, 2);
		memcpy(&system_report_prev, &system_report,
			sizeof(system_report));
		sent = true;
	}

	memset(&system_report, 0, sizeof(system_report));

	return sent;
}

bool
send_gamepad_report(void)
{
	bool sent;

	sent = false;
	if (memcmp(&gamepad_report, &gamepad_report_prev,
			sizeof(gamepad_report))) {
		tud_hid_n_gamepad_report(INST_HID_MISC, REPORT_ID_GAMEPAD,
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
	case REPORT_ID_MOUSE:
		return send_mouse_report();
	case REPORT_ID_CONSUMER:
		return send_consumer_report();
	case REPORT_ID_SYSTEM:
		return send_system_report();
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

bool
hid_ready()
{
	return tud_hid_n_ready(INST_HID_KBD)
		&& tud_hid_n_ready(INST_HID_MISC);
}

void
tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol)
{
	if (protocol == HID_PROTOCOL_BOOT) {
		led_rgb = SOFT_YELLOW;
	} else {
		led_rgb = SOFT_PURPLE;
	}
	led_mode = LED_ON;
	led_reset = true;
}

void
tud_hid_report_complete_cb(uint8_t instance,
	uint8_t const *report, uint8_t len)
{
	if (report[0] >= REPORT_ID_MIN)
		send_next_hid_report(report[0] + 1);
}

void
hid_init(void)
{
}

void
hid_force_release(uint x, uint y)
{
	process_keyrelease(keysyms[y][x], x, y);
	keysyms[y][x] = KC_NO;
}

void
hid_switch_layer_with_key(uint8_t layer, uint x, uint y)
{
	active_layers_push(layer, y * KEY_COLS + x);
	keysyms[y][x] = SW(layer);
	seen_mat[y][x] = true;
}

void
hid_send_macro(const uint32_t *keysyms, uint cnt)
{
	static const uint mx = MACRO_X, my = MACRO_Y;
	struct hid_keyboard_report tmp;
	uint32_t start_ms;
	uint i, k;

	/* NOTE: layer switching is not supported for macros (not needed),
	 * to preserve the current layers we reference a key which is not
	 * in-use to prevent accidentally unmapping layers on release */

	macro_held_reset();

	active_mods = 0;
	active_weak_mods = 0;
	macro_running = true;
	memset(&keyboard_report, 0, sizeof(keyboard_report));
	memset(&keyboard_report_prev, 0, sizeof(keyboard_report));
	memset(&seen_mat, 0, sizeof(seen_mat));
	while (!hid_ready()) tud_task();

	for (i = 0; i < cnt; i++) {
		if (IS_MACRO_DELAY(keysyms[i])) {
			start_ms = board_millis();
			while (board_millis() - start_ms < TO_DELAY(keysyms[i]))
				tud_task();
			continue;
		}

		memset(&keyboard_report, 0, sizeof(keyboard_report));

		if (IS_MACRO_RELEASE(keysyms[i]))
			macro_held_pop(TO_SYM(keysyms[i]));

		for (k = 0; k < i; k++) {
			if (!IS_MACRO_HOLD(keysyms[k]))
				continue;
			if (macro_held_find(TO_SYM(keysyms[k])))
				process_keydown(TO_SYM(keysyms[k]), mx, my);
		}

		if (IS_MACRO_PRESS(keysyms[i])) {
			keyboard_report.mods = active_weak_mods | active_mods;
			memcpy(&tmp, &keyboard_report, sizeof(keyboard_report));
			process_keypress(TO_SYM(keysyms[i]), mx, my);
			process_keydown(TO_SYM(keysyms[i]), mx, my);
		} else if (IS_MACRO_HOLD(keysyms[i])) {
			macro_held_push(TO_SYM(keysyms[i]));
		} else if (IS_MACRO_RELEASE(keysyms[i])) {
			process_keyrelease(TO_SYM(keysyms[i]), mx, my);
			process_keyup(TO_SYM(keysyms[i]), mx, my);
		}

		send_keyboard_report();
		while (!hid_ready()) tud_task();

		if (IS_MACRO_PRESS(keysyms[i])) {
			memcpy(&keyboard_report, &tmp, sizeof(keyboard_report));
			send_keyboard_report();
			while (!hid_ready()) tud_task();
		}
	}

	memset(&keyboard_report, 0, sizeof(keyboard_report));
	send_keyboard_report();
	while (!hid_ready()) tud_task();

	macro_running = false;
}

void
hid_task(void)
{
	update_report();
	if (tud_hid_n_ready(INST_HID_KBD))
		send_keyboard_report();
	if (tud_hid_n_ready(INST_HID_MISC))
		send_next_hid_report(REPORT_ID_MIN);
}

