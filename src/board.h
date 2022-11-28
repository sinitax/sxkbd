#pragma once

#include "neopix.h"

#define ONBOARD_LED_PIN 25

#define REPORT_ID_MIN REPORT_ID_KEYBOARD
enum {
	REPORT_ID_KEYBOARD = 1,
	REPORT_ID_MOUSE,
	REPORT_ID_CONSUMER_CONTROL,
	REPORT_ID_MAX
};

extern struct neopix onboard_led;

extern const uint32_t **keymap_layers;

