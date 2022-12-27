#pragma once

#include "keymat.h"

#include <stdbool.h>
#include <stdint.h>

#define MACRO_HOLD_MAX 32

#define REPORT_ID_MIN REPORT_ID_KEYBOARD
enum {
	REPORT_ID_KEYBOARD = 1,
	REPORT_ID_CONSUMER,
	REPORT_ID_SYSTEM,
	REPORT_ID_MOUSE,
	REPORT_ID_GAMEPAD,
	REPORT_ID_MAX
};

void hid_init(void);
void hid_force_release(uint x, uint y);
void hid_switch_layer_with_key(uint8_t layer, uint x, uint y);
void hid_send_macro(const uint32_t *keysyms, uint cnt);
void hid_task(void);
