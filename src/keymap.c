#include "keymap.h"
#include "hid/keyboard.h"
#include "keysym.h"
#include "keysym/consumer.h"
#include "keysym/keyboard_de.h"
#include "keysym/keyboard_us.h"
#include "hid.h"
#include "board.h"
#include "util.h"

#define KEYMAP( \
		K11, K12, K13, K14, K15, K16, \
		K21, K22, K23, K24, K25, K26, \
		K31, K32, K33, K34, K35, K36, \
		               K44, K45, K46, \
		K51, K52, K53, K54, K55, K56, \
		K61, K62, K63, K64, K65, K66, \
		K71, K72, K73, K74, K75, K76, \
		K81, K82, K83 \
	) { \
		{ K11, K12, K13, K14, K15, K16 }, \
		{ K21, K22, K23, K24, K25, K26 }, \
		{ K31, K32, K33, K34, K35, K36 }, \
		{ 0x0, 0x0, 0x0, K44, K45, K46 }, \
		{ K56, K55, K54, K53, K52, K51 }, \
		{ K66, K65, K64, K63, K62, K61 }, \
		{ K76, K75, K74, K73, K72, K71 }, \
		{ 0x0, 0x0, 0x0, K83, K82, K81 }  \
	}

/* KEY INDEX LOOKUP:
 * x0y0    , x1y0    , x2y0    , x3y0    , x4y0    , x5y0    ,
 * x0y1    , x1y1    , x2y1    , x3y1    , x4y1    , x5y1    ,
 * x0y2    , x1y2    , x2y2    , x3y2    , x4y2    , x5y2    ,
 *                               x3y3    , x4y3    , x5y3    ,
 *
 * x5y4    , x4y4    , x3y4    , x2y4    , x1y4    , x0y4    ,
 * x5y5    , x4y5    , x3y5    , x2y5    , x1y5    , x0y5    ,
 * x5y6    , x4y6    , x3y6    , x2y6    , x1y6    , x0y6    ,
 * x5y7    , x4y7    , x3y7
 */


#define LAYER_BASE_DE KEYMAP(                                       \
	KC_ESC  , DE_Q    , DE_W    , DE_F    , DE_P    , DE_B    , \
	SX(QUSW), DE_A    , DE_R    , DE_S    , DE_T    , DE_G    , \
	KC_LSFT , DE_Z    , DE_X    , DE_C    , DE_D    , DE_V    , \
	                              KC_LGUI , KC_LALT , SW(SHRT), \
	                                                            \
	DE_J    , DE_L    , DE_U    , DE_Y    , DE_HASH , DE_SS   , \
	DE_M    , DE_N    , DE_E    , DE_I    , DE_O    , SW(SPEC), \
	DE_K    , DE_H    , DE_COMM , DE_DOT  , DE_MINS , SW(MISC), \
	SW(NUMS), KC_SPC  , KC_LCTL                                 \
)

#define LAYER_QUIK_DE KEYMAP(                                       \
	SW(META), KC_PGDN , KC_PGUP , KC_UP   , DE_DLR  , DE_QUOT , \
	_______ , _______ , KC_LEFT , KC_DOWN , KC_RGHT , DE_EXLM , \
	_______ , _______ , DE_PIPE , DE_LESS , DE_GRTR , DE_BSLS , \
	                              _______ , _______ , CS(BASE), \
	                                                            \
	DE_TILD , DE_LPRN , DE_RPRN , DE_DQUO , DE_GRV  , DE_QUES , \
	KC_BSPC , KC_ENT  , DE_SLSH , DE_SCLN , DE_ASTR , DE_CIRC , \
	DE_PERC , DE_AMPR , DE_MINS , DE_PLUS , DE_EQL  , _______ , \
	KC_TAB  , _______ , SW(QUIX)                                \
)

#define LAYER_QUIX_DE KEYMAP(                                       \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , DE_LCBR , _______ , DE_RCBR , _______ , \
	_______ , _______ , _______ , DE_LBRC , DE_RBRC , _______ , \
	                              _______ , _______ , _______ , \
	                                                            \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______                                 \
)

#define LAYER_SHRT_DE KEYMAP(                                       \
	_______ , G(KC_1) , G(KC_2) , G(KC_3) , G(KC_4) , G(KC_5) , \
	CS(BASE), A(KC_1) , A(KC_2) , A(KC_3) , A(KC_4) , A(KC_5) , \
	_______ ,G(KC_TAB),G(DE_DOT), A(DE_B) , A(DE_F) , KC_SPC  , \
	                              _______ , _______ , _______ , \
	                                                            \
	G(KC_6) , G(KC_7) , G(KC_8) , G(KC_9) , G(KC_0) , _______ , \
	A(KC_6) , A(KC_7) , A(KC_8) , A(KC_9) , A(KC_0) , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______                                 \
)

#define LAYER_NUMS_DE KEYMAP(                                       \
	_______ , DE_1    , DE_2    , DE_3    , DE_4    , DE_5    , \
	KC_TAB  , _______ , DE_A    , DE_B    , DE_C    , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	                              _______ , _______ , _______ , \
	                                                            \
	DE_6    , DE_7    , DE_8    , DE_9    , DE_0    , _______ , \
	_______ , DE_D    , DE_E    , DE_F    , _______ , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______                                 \
)

#define LAYER_SPEC_DE KEYMAP(                                       \
	_______ , _______ , DE_AT   , _______ , _______ , _______ , \
	_______ , DE_ADIA , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	                              _______ , _______ , _______ , \
	                                                            \
	_______ , _______ , DE_UDIA , DE_ACUT , _______ , KC_DEL  , \
	_______ , _______ , _______ , _______ , DE_ODIA , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______                                 \
)

#define LAYER_META_DE KEYMAP(                                       \
	_______ , SX(KVM1), SX(KVM2), SX(KVM3), SX(KVM4), _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , KC_F6   , \
	                              RB(BASE), RB(GM1B), RB(GM2B), \
	                                                            \
	_______ , KS_VOLD , KS_VOLU , KS_MUTE , _______ , _______ , \
	_______ , KS_MPRV , KS_MNXT , KS_MPLY , _______ , _______ , \
	KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  , \
	_______ , _______ , _______                                 \
)

#define LAYER_MISC_DE KEYMAP(                                       \
	XXXXXXX , SX(TTY1), SX(TTY2), SX(TTY3), SX(TTY4), SX(TTY5), \
	XXXXXXX , SX(TTY6), SX(TTY7), SX(TTY8), SX(TTY9),SX(TTY10), \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	                              XXXXXXX , XXXXXXX , XXXXXXX , \
	                                                            \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX                                 \
)

#define LAYER_GM1B_DE KEYMAP(                                       \
	KC_ESC  , DE_Q    , DE_W    , DE_E    , DE_R    , DE_T    , \
	SW(GM1N), DE_A    , DE_S    , DE_D    , DE_F    , DE_G    , \
	KC_LCTL , DE_Z    , DE_X    , DE_C    , DE_V    , DE_B    , \
	                              KC_LSFT , KC_LALT , KC_SPC  , \
	                                                            \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX                                 \
)

#define LAYER_GM1N_DE KEYMAP(                                       \
	SW(META), KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , \
	XXXXXXX , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	                              XXXXXXX , KC_TAB  , XXXXXXX , \
	                                                            \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX                                 \
)

#define LAYER_GM2B_DE KEYMAP(                                       \
	KC_ESC  , DE_Q    , DE_W    , DE_E    , DE_R    , DE_T    , \
	SW(GM2N), DE_A    , DE_S    , DE_D    , DE_F    , DE_G    , \
	KC_LCTL , DE_Z    , DE_X    , DE_C    , DE_V    , DE_B    , \
	                              SW(GM2E), KC_LALT , KC_SPC  , \
	                                                            \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX                                 \
)

#define LAYER_GM2N_DE KEYMAP(                                       \
	SW(META), KC_1    , DE_W    , KC_2    , KC_3    , DE_6    , \
	XXXXXXX , DE_A    , DE_S    , DE_D    , KC_4    , DE_7    , \
	XXXXXXX , KC_LSFT , DE_9    , DE_0    , KC_5    , DE_8 , \
	                              XXXXXXX , KC_TAB  , XXXXXXX , \
	                                                            \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX                                 \
)

#define LAYER_GM2E_DE KEYMAP(                                       \
	DE_J    , DE_L    , DE_W    , DE_Y    , DE_HASH , DE_SS   , \
	DE_M    , DE_A    , DE_S    , DE_D    , DE_O    , DE_U    , \
	DE_K    , DE_H    , DE_COMM , DE_DOT  , DE_MINS , DE_N    , \
	                              XXXXXXX , XXXXXXX , XXXXXXX , \
	                                                            \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , \
	XXXXXXX , XXXXXXX , XXXXXXX                                 \
)

enum {
	BASE, /* base */
	QUIK, /* symbols + nav */
	QUIX, /* quick xtras */
	SHRT, /* shortcuts */
	LCTL, /* left ctrl */
	NUMS, /* numbers */
	SPEC, /* specials */
	META, /* functions */
	MISC,
	GM1B, /* gaming 1 base */
	GM1N, /* gaming 1 nums */
	GM2B, /* gaming 2 base */
	GM2N, /* gaming 2 nums */
	GM2E, /* gaming 2 extended */
};

enum {
	KVM1,
	KVM2,
	KVM3,
	KVM4,
	QMSW,
	QUSW,
	TTY1,
	TTY2,
	TTY3,
	TTY4,
	TTY5,
	TTY6,
	TTY7,
	TTY8,
	TTY9,
	TTY10,
};

const uint32_t keymap_layers_de[][KEY_ROWS][KEY_COLS] = {
	[BASE] = LAYER_BASE_DE,
	[QUIK] = LAYER_QUIK_DE,
	[QUIX] = LAYER_QUIX_DE,
	[SHRT] = LAYER_SHRT_DE,
	[NUMS] = LAYER_NUMS_DE,
	[SPEC] = LAYER_SPEC_DE,
	[META] = LAYER_META_DE,
	[MISC] = LAYER_MISC_DE,
	[GM1B] = LAYER_GM1B_DE,
	[GM1N] = LAYER_GM1N_DE,
	[GM2B] = LAYER_GM2B_DE,
	[GM2N] = LAYER_GM2N_DE,
	[GM2E] = LAYER_GM2E_DE,

};
const uint32_t keymap_layers_de_count = ARRLEN(keymap_layers_de);

const uint32_t (*keymap_layers)[KEY_ROWS][KEY_COLS] = keymap_layers_de;
uint32_t keymap_layers_count = keymap_layers_de_count;

static const uint32_t macro_kvm1[] = {
	KC_LEFT_CTRL, KC_LEFT_CTRL, KC_1
};

static const uint32_t macro_kvm2[] = {
	KC_LEFT_CTRL, KC_LEFT_CTRL, KC_2
};

static const uint32_t macro_kvm3[] = {
	KC_HOME
};

static const uint32_t macro_kvm4[] = {
	HOLD(KC_LEFT_SHIFT), KC_HOME
};

static const uint32_t macro_qemu_switch[] = {
	HOLD(KC_LEFT_CTRL), KC_RIGHT_CTRL
};

static const uint32_t ttysw_lut[] = {
	KC_F1, KC_F2, KC_F3, KC_F4, KC_F5,
	KC_F6, KC_F7, KC_F8, KC_F9, KC_F10
};
static uint32_t macro_ttysw[] = {
	HOLD(KC_LEFT_CTRL), HOLD(KC_LEFT_ALT), KC_F1,
};

void
process_user_keypress(uint8_t sym, uint x, uint y)
{
	switch (sym) {
	case KVM1:
		hid_send_macro(macro_kvm1, ARRLEN(macro_kvm1));
		break;
	case KVM2:
		hid_send_macro(macro_kvm2, ARRLEN(macro_kvm2));
		break;
	case KVM3:
		hid_send_macro(macro_kvm3, ARRLEN(macro_kvm3));
		break;
	case KVM4:
		hid_send_macro(macro_kvm4, ARRLEN(macro_kvm4));
		break;
	case QMSW:
		hid_send_macro(macro_qemu_switch, ARRLEN(macro_qemu_switch));
		break;
	case QUSW:
		if (keymat[7][3]) {
			hid_force_release(3, 7);
			hid_switch_layer_with_key(QUIX, x, y);
		} else {
			hid_switch_layer_with_key(QUIK, x, y);
		}
		break;
	case TTY1...TTY10:
		macro_ttysw[2] = ttysw_lut[sym - TTY1];
		hid_send_macro(macro_ttysw, ARRLEN(macro_ttysw));
		break;
	}

}

void
process_user_keyrelease(uint8_t sym, uint x, uint y)
{

}
