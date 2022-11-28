#include "keymap.h"
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
		K51, K52, K53, K54, K55, K56, \
		K61, K62, K63, K64, K65, K66, \
		K31, K32, K33, K34, K35, K36, \
		K31, K32, K33, 0x0, 0x0, 0x0, \
	}

enum {
	BA /* BASE */
};

static const uint32_t layer_base_de[] = KEYMAP(
	_______, DE_Q   , DE_W   , DE_F   , DE_P   , DE_B   ,
	_______, DE_A   , DE_R   , DE_S   , DE_T   , DE_G   ,
	_______, DE_Z   , DE_X   , DE_C   , DE_D   , DE_V   ,
	                           KC_LALT, KC_LGUI, _______,

	_______, DE_J   , DE_L   , DE_U   , DE_Y   , _______,
	_______, DE_N   , DE_E   , DE_I   , DE_O   , _______,
	_______, DE_H   , DE_SCLN, DE_DOT , DE_MINS, _______,
	_______, _______, _______
);

const uint32_t *keymap_layers_de[] = {
	[BA] = layer_base_de
};
const uint32_t keymap_layers_de_count = ARRLEN(keymap_layers_de);
