#pragma once

#include "keycode.h"

#define XXXXXXX KC_NO
#define _______ KC_TRNS

#define MASK(hi, lo) ((1U << (hi)) - (1U << (lo)))

#define IS_KC(x)    (!((x) & ~MASK(B_TOGGLE, 0)))
#define IS_CTRL(x)    ((x) & (1 << B_CTRL))
#define IS_SHIFT(x)   ((x) & (1 << B_SHIFT))
#define IS_ALT(x)     ((x) & (1 << B_ALT))
#define IS_GUI(x)     ((x) & (1 << B_GUI))
#define IS_RIGHT(x)   ((x) & (1 << B_RIGHT))
#define IS_TOGGLE(x)  ((x) & (1 << B_TOGGLE))
#define IS_SWITCH(x)  ((x) & (1 << B_SWITCH))
#define IS_USER(x)    ((x) & (1 << B_USER))

#define IS_LEFT_CTRL(x)   (IS_CTRL(x) && !IS_RIGHT(x))
#define IS_RIGHT_CTRL(x)  (IS_CTRL(x) && IS_RIGHT(x))
#define IS_LEFT_SHIFT(x)  (IS_SHIFT(x) && !IS_RIGHT(x))
#define IS_RIGHT_SHIFT(x) (IS_SHIFT(x) && IS_RIGHT(x))
#define IS_LEFT_ALT(x)    (IS_ALT(x) && !IS_RIGHT(x))
#define IS_RIGHT_ALT(x)   (IS_ALT(x) && IS_RIGHT(x))
#define IS_LEFT_GUI(x)    (IS_GUI(x) && !IS_RIGHT(x))
#define IS_RIGHT_GUI(x)   (IS_GUI(x) && IS_RIGHT(x))

#define IS_MOD(x) ((x) & MASK(B_TOGGLE, 8))

#define TO_KC(x)    ((x) & 0xFF)
#define TO_SYM(x)   ((x) & 0xFF)
#define TO_LAYER(x) ((x) & 0xFF)

#define LCTL(x) ((x) | (1 << B_CTRL))
#define LSFT(x) ((x) | (1 << B_SHIFT))
#define LALT(x) ((x) | (1 << B_ALT))
#define LGUI(x) ((x) | (1 << B_GUI))
#define RCTL(x) (LCTL(x) | (1 << B_RIGHT))
#define RSFT(x) (LSFT(x) | (1 << B_RIGHT))
#define RALT(x) (LALT(x) | (1 << B_RIGHT))
#define RGUI(x) (LGUI(x) | (1 << B_RIGHT))

#define SW(x) ((x) | (1 << B_SWITCH))
#define TO(x) ((x) | (1 << B_TOGGLE))
#define SX(x) ((x) | (1 << B_USER))

#define LOPT(x) LALT(x)
#define LCMD(x) LGUI(x)
#define LWIN(x) LGUI(x)
#define ALGR(x) RALT(x)
#define ROPT(x) RALT(x)
#define RCMD(x) RGUI(x)

#define C(x) LCTL(x)
#define S(x) LSFT(x)
#define A(x) LALT(x)
#define G(x) LGUI(x)

#define CS(x) C(SW(x))
#define GS(x) G(SW(x))

enum {
	B_CTRL = 8,
	B_SHIFT,
	B_ALT,
	B_GUI,
	B_RIGHT,
	B_TOGGLE,
	B_SWITCH,
	B_USER
};

