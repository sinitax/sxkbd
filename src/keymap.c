#include "keymap.h"
#include "keycode.h"
#include "keysym.h"
#include "keysym_de.h"
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
		K11, K12, K13, K14, K15, K16, \
		K21, K22, K23, K24, K25, K26, \
		K31, K32, K33, K34, K35, K36, \
		0x0, 0x0, 0x0, K44, K45, K46, \
		K56, K55, K54, K53, K52, K51, \
		K66, K65, K64, K63, K62, K61, \
		K76, K75, K74, K73, K72, K71, \
		0x0, 0x0, 0x0, K83, K82, K81, \
	}

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

static const uint32_t layer_base_de[] = KEYMAP(
	KC_ESC  , DE_Q    , DE_W    , DE_F    , DE_P    , DE_B    ,
	SW(QUIK), DE_A    , DE_R    , DE_S    , DE_T    , DE_G    ,
	KC_LSFT , DE_Z    , DE_X    , DE_C    , DE_D    , DE_V    ,
	                              KC_LGUI , KC_LALT , SW(SHRT),

	DE_J    , DE_L    , DE_U    , DE_Y    , DE_QUOT , DE_PLUS ,
	DE_M    , DE_N    , DE_E    , DE_I    , DE_O    , SW(SPEC),
	DE_K    , DE_H    , DE_COMM , DE_DOT  , DE_MINS , XXXXXXX ,
	SW(NUMS), KC_SPC  , KC_LCTL
);

static const uint32_t layer_quik_de[] = KEYMAP(
	SW(META), KC_PGDN , KC_PGUP , KC_UP   , DE_DLR  , DE_QUOT ,
	_______ , _______ , KC_LEFT , KC_DOWN , KC_RGHT , DE_EXLM ,
	_______ , _______ , DE_PIPE , DE_LESS , DE_GRTR , DE_BSLS ,
	                              _______ , _______ , CS(BASE),

	DE_TILD , DE_LPRN , DE_RPRN , DE_DQUO , DE_GRV  , DE_QUES ,
	KC_BSPC , KC_ENT  , DE_SLSH , DE_SCLN , DE_ASTR , DE_CIRC ,
	DE_PERC , DE_AMPR , DE_MINS , DE_PLUS , DE_EQL  , _______ ,
	KC_TAB  , _______ , SW(QUIX)
);

static const uint32_t layer_quix_de[] = KEYMAP(
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	                              _______ , _______ , _______ ,

	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______
);

static const uint32_t layer_shrt_de[] = KEYMAP(
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	                              _______ , _______ , _______ ,

	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______
);

static const uint32_t layer_nums_de[] = KEYMAP(
	_______ , DE_1    , DE_2    , DE_3    , DE_4    , DE_5    ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	                              _______ , _______ , _______ ,

	_______ , DE_6    , DE_7    , DE_8    , DE_9    , DE_0    ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______
);

static const uint32_t layer_spec_de[] = KEYMAP(
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	                              _______ , _______ , _______ ,

	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______
);

static const uint32_t layer_meta_de[] = KEYMAP(
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	                              _______ , _______ , _______ ,

	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______ , _______ , _______ , _______ ,
	_______ , _______ , _______
);

const uint32_t *keymap_layers_de[] = {
	[BASE] = layer_base_de,
	[QUIK] = layer_quik_de,
	[QUIX] = layer_quix_de,
	[SHRT] = layer_shrt_de,
	[NUMS] = layer_nums_de,
	[SPEC] = layer_spec_de,
	[META] = layer_meta_de

};
const uint32_t keymap_layers_de_count = ARRLEN(keymap_layers_de);

const uint32_t **keymap_layers = keymap_layers_de;
uint32_t keymap_layers_count = keymap_layers_de_count;
