#pragma once

#include "keymat.h"

#include <stdbool.h>
#include <stdint.h>

#define REPORT_ID_MIN REPORT_ID_KEYBOARD
enum {
	REPORT_ID_KEYBOARD = 1,
	REPORT_ID_MOUSE,
	REPORT_ID_CONSUMER_CONTROL,
	REPORT_ID_MAX
};

void hid_init(void);
void hid_force_release(uint x, uint y);
void hid_switch_layer_with_key(uint8_t layer, uint x, uint y);
void hid_task(void);
