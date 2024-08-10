#pragma once

#include "util.h"

#include <stdint.h>

#define KEY_ROWS_HALF 4
#define KEY_ROWS (KEY_ROWS_HALF * 2)
#define KEY_COLS 6
#define KEY_COUNT (KEY_ROWS * KEY_COLS)

#define KEYMAT_HALF(keymat, side) ((side) == LEFT ? &(keymat)[0] : &(keymat)[KEY_ROWS_HALF])

void keymat_init(void);
bool keymat_scan(void);
uint32_t keymat_encode_half(int side);
void keymat_decode_half(int side, uint32_t mask);

void keymat_debug(void);

extern bool keymat_prev[KEY_ROWS][KEY_COLS];
extern bool keymat[KEY_ROWS][KEY_COLS];

