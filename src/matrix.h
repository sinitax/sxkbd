#pragma once

#include "util.h"

#include <stdint.h>

#define KEY_ROWS 4
#define KEY_COLS 6
#define KEY_COUNT (KEY_ROWS * KEY_COLS)

void matrix_init(void);
void scan_matrix(void);

extern bool prev_state_matrix[KEY_COUNT];
extern bool state_matrix[KEY_COUNT];
extern uint32_t sym_matrix[KEY_COUNT];

