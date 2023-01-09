#pragma once

#define XXXXXXX KC_NO
#define _______ KC_TRNS

#define MASK(hi, lo) ((1U << (hi)) - (1U << (lo)))

#define IS_KC(x)    (!((x) & ~MASK(B_TOGGLE, 0)))
#define IS_CTRL(x)    ((x) & (1U << B_CTRL))
#define IS_SHIFT(x)   ((x) & (1U << B_SHIFT))
#define IS_ALT(x)     ((x) & (1U << B_ALT))
#define IS_GUI(x)     ((x) & (1U << B_GUI))
#define IS_RIGHT(x)   ((x) & (1U << B_RIGHT))
#define IS_TOGGLE(x)  ((x) & (1U << B_TOGGLE))
#define IS_REBASE(x)  ((x) & (1U << B_REBASE))
#define IS_SWITCH(x)  ((x) & (1U << B_SWITCH))
#define IS_SPECIAL(x) ((x) & (1U << B_SPECIAL))
#define IS_USER(x)    ((x) & (1U << B_USER))

#define IS_MOUSE(x)    (IS_SPECIAL(x) && (TO_SPECIAL(x) == S_MOUSE))
#define IS_SYSTEM(x)   (IS_SPECIAL(x) && (TO_SPECIAL(x) == S_SYSTEM))
#define IS_CONSUMER(x) (IS_SPECIAL(x) && (TO_SPECIAL(x) == S_CONSUMER))

#define IS_MACRO_HOLD(x)    ((x) & (1U << B_MACRO_HOLD))
#define IS_MACRO_RELEASE(x) ((x) & (1U << B_MACRO_RELEASE))
#define IS_MACRO_DELAY(x)   ((x) & (1U << B_MACRO_DELAY))
#define IS_MACRO_PRESS(x)   (!IS_MACRO_HOLD(x) && !IS_MACRO_RELEASE(x) \
		&& !IS_MACRO_DELAY(x))

#define IS_LEFT_CTRL(x)   (IS_CTRL(x) && !IS_RIGHT(x))
#define IS_RIGHT_CTRL(x)  (IS_CTRL(x) && IS_RIGHT(x))
#define IS_LEFT_SHIFT(x)  (IS_SHIFT(x) && !IS_RIGHT(x))
#define IS_RIGHT_SHIFT(x) (IS_SHIFT(x) && IS_RIGHT(x))
#define IS_LEFT_ALT(x)    (IS_ALT(x) && !IS_RIGHT(x))
#define IS_RIGHT_ALT(x)   (IS_ALT(x) && IS_RIGHT(x))
#define IS_LEFT_GUI(x)    (IS_GUI(x) && !IS_RIGHT(x))
#define IS_RIGHT_GUI(x)   (IS_GUI(x) && IS_RIGHT(x))

#define IS_MOD(x) ((x) & MASK(B_TOGGLE, B_CTRL))

#define TO_SPECIAL(x) (((x) >> B_SPECIAL_SEL) & 0b11)
#define TO_KC(x)    ((x) & 0xFF)
#define TO_USER(x)  ((x) & 0xFF)
#define TO_LAYER(x) ((x) & 0xFF)
#define TO_SYM(x)   ((x) & 0xFFFF)
#define TO_DELAY(x) ((x) & 0xFFFF)

#define SPECIAL(x, g) ((x) | (1U << B_SPECIAL) | ((g) << B_SPECIAL_SEL))

#define LCTL(x) ((x) | (1 << B_CTRL))
#define LSFT(x) ((x) | (1 << B_SHIFT))
#define LALT(x) ((x) | (1 << B_ALT))
#define LGUI(x) ((x) | (1 << B_GUI))
#define RIGH(x) ((x) | (1 << B_RIGHT))

#define HOLD(x)    ((x) | (1 << B_MACRO_HOLD))
#define RELEASE(x) ((x) | (1 << B_MACRO_RELEASE))
#define DELAY(x)   ((x) | (1 << B_MACRO_DELAY))

#define SW(x) ((x) | (1 << B_SWITCH))
#define TO(x) ((x) | (1 << B_TOGGLE))
#define RB(x) ((x) | (1 << B_REBASE))
#define SX(x) ((x) | (1 << B_USER))

#define RCTL(x) RIGH(LCTL(x))
#define RSFT(x) RIGH(LSFT(x))
#define RALT(x) RIGH(LALT(x))
#define RGUI(x) RIGH(LGUI(x))

#define LOPT(x) LALT(x)
#define LCMD(x) LGUI(x)
#define LWIN(x) LGUI(x)
#define ALGR(x) RALT(x)
#define ROPT(x) RALT(x)
#define RCMD(x) RGUI(x)

#define CS(x) C(SW(x))
#define AS(x) A(SW(x))
#define GS(x) G(SW(x))

#define C(x) LCTL(x)
#define S(x) LSFT(x)
#define A(x) LALT(x)
#define G(x) LGUI(x)

enum {
	S_CONSUMER,
	S_MOUSE,
	S_SYSTEM
};

enum {
	B_SPECIAL_SEL = 6,
	B_CTRL = 8,
	B_SHIFT,
	B_ALT,
	B_GUI,
	B_RIGHT,
	B_TOGGLE,
	B_REBASE,
	B_SWITCH,
	B_SPECIAL,
	B_USER,
	B_MACRO_HOLD,
	B_MACRO_RELEASE,
	B_MACRO_DELAY /* >= 16 */
};

