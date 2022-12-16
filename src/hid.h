#pragma once

#include <stdbool.h>
#include <stdint.h>

void hid_init(void);
bool hid_gen_report(uint8_t *keycode);
void hid_task(void);
