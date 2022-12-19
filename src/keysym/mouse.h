#pragma once

#include "keysym.h"

#define KS_MS_U KS_MS_UP
#define KS_MS_D KS_MS_DOWN
#define KS_MS_L KS_MS_LEFT
#define KS_MS_R KS_MS_RIGHT
#define KS_BTN1 KS_MS_BTN1
#define KS_BTN2 KS_MS_BTN2
#define KS_BTN3 KS_MS_BTN3
#define KS_BTN4 KS_MS_BTN4
#define KS_BTN5 KS_MS_BTN5
#define KS_BTN6 KS_MS_BTN6
#define KS_BTN7 KS_MS_BTN7
#define KS_BTN8 KS_MS_BTN8
#define KS_WH_U KS_MS_WH_UP
#define KS_WH_D KS_MS_WH_DOWN
#define KS_WH_L KS_MS_WH_LEFT
#define KS_WH_R KS_MS_WH_RIGHT
#define KS_ACL0 KS_MS_ACCEL0
#define KS_ACL1 KS_MS_ACCEL1
#define KS_ACL2 KS_MS_ACCEL2

enum keysym_mouse {
	/* Mouse Buttons */
	KC_MS_UP = KEYSYM_MOUSE_MASK,
	KC_MS_DOWN,
	KC_MS_LEFT,
	KC_MS_RIGHT,
	KC_MS_BTN1,
	KC_MS_BTN2,
	KC_MS_BTN3,
	KC_MS_BTN4,
	KC_MS_BTN5,
	KC_MS_BTN6,
	KC_MS_BTN7,
	KC_MS_BTN8,

	/* Mouse Wheel */
	KC_MS_WH_UP,
	KC_MS_WH_DOWN,
	KC_MS_WH_LEFT,
	KC_MS_WH_RIGHT,

	/* Acceleration */
	KC_MS_ACCEL0,
	KC_MS_ACCEL1,
	KC_MS_ACCEL2
};
