#pragma once

#include "util.h"

#include <stdint.h>

#define KEY_ROWS 4
#define KEY_COLS 6
#define KEY_COUNT (KEY_ROWS * KEY_COLS)

#define KEYMAT_HALF(side) ((side) == LEFT ? &keymat[0] : &keymat[KEY_ROWS])

void keymat_init(void);
void keymat_scan(void);
uint32_t keymat_encode_half(int side);
void keymat_decode_half(int side, uint32_t);

extern bool keymat_prev[KEY_ROWS * 2][KEY_COLS];
extern bool keymat[KEY_ROWS * 2][KEY_COLS];

