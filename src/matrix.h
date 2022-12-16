#pragma once

#include "util.h"

#include <stdint.h>

#define KEY_ROWS 4
#define KEY_COLS 6
#define KEY_COUNT (KEY_ROWS * KEY_COLS)

#define MAT_OFFSET(side) ((side) == LEFT ? 0 : KEY_COUNT)

void matrix_init(void);
void scan_matrix(void);
uint32_t matrix_encode_half(int side);
void matrix_decode_half(int side, uint32_t);

extern bool prev_state_matrix[KEY_COUNT * 2];
extern bool state_matrix[KEY_COUNT * 2];

