#include "keymap.h"
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
	DE_K    , DE_H    , DE_COMM , DE_DOT  , DE_MINS , XXXXXXX , \
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
	_______ ,G(KC_TAB),G(DE_DOT), A(DE_B) , A(DE_F) ,A(KC_SPC), \
	                              _______ , _______ , _______ , \
	                                                            \
	G(KC_6) , G(KC_7) , G(KC_8) , G(KC_9) , G(KC_0) , _______ , \
	A(KC_6) , A(KC_7) , A(KC_8) , A(KC_9) , A(KC_0) , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______                                 \
)

#define LAYER_NUMS_DE KEYMAP(                                       \
	_______ , DE_1    , DE_2    , DE_3    , DE_4    , DE_5    , \
	KC_TAB  , _______ , _______ , _______ , _______ , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	                              _______ , _______ , _______ , \
	                                                            \
	DE_6    , DE_7    , DE_8    , DE_9    , DE_0    , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
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
	_______ , SX(KVM1), SX(KVM2), _______ , _______ , _______ , \
	_______ , _______ , _______ , _______ , _______ , _______ , \
	KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , KC_F6   , \
	                              _______ , _______ , _______ , \
	                                                            \
	_______ , KS_VOLD , KS_VOLU , KS_MUTE , _______ , _______ , \
	_______ , KS_MPRV , KS_MNXT , KS_MPLY , _______ , _______ , \
	KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  , \
	_______ , _______ , _______                                 \
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
};

enum {
	KVM1,
	KVM2,
	QUSW,
};

const uint32_t keymap_layers_de[][KEY_ROWS][KEY_COLS] = {
	[BASE] = LAYER_BASE_DE,
	[QUIK] = LAYER_QUIK_DE,
	[QUIX] = LAYER_QUIX_DE,
	[SHRT] = LAYER_SHRT_DE,
	[NUMS] = LAYER_NUMS_DE,
	[SPEC] = LAYER_SPEC_DE,
	[META] = LAYER_META_DE

};
const uint32_t keymap_layers_de_count = ARRLEN(keymap_layers_de);

const uint32_t (*keymap_layers)[KEY_ROWS][KEY_COLS] = keymap_layers_de;
uint32_t keymap_layers_count = keymap_layers_de_count;

void
process_user_keypress_new(uint8_t sym, uint x, uint y)
{
	switch (sym) {
	case KVM1:
		break;
	case KVM2:
		break;
	case QUSW:
		if (keymat[7][3]) {
			hid_force_release(3, 7);
			hid_switch_layer_with_key(QUIX, x, y);
		} else {
			hid_switch_layer_with_key(QUIK, x, y);
		}
		break;
	}

}

void
process_user_keyrelease_new(uint8_t sym, uint x, uint y)
{

}
