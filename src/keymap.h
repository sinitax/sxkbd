#pragma once

#include "keymat.h"

#include "pico/types.h"

#include <stdint.h>

void process_user_keypress(uint8_t sym, uint x, uint y);
void process_user_keyrelease(uint8_t sym, uint x, uint y);

extern const uint32_t keymap_layers_de[][KEY_ROWS][KEY_COLS];
extern const uint32_t keymap_layers_de_count;

extern const uint32_t (*keymap_layers)[KEY_ROWS][KEY_COLS];
extern uint32_t keymap_layers_count;
