#pragma once

#include "keysym.h"

#include <stdint.h>

/* Generic Desktop Page (0x01) */
#define KS_PWR  KS_SYSTEM_POWER
#define KS_SLEP KS_SYSTEM_SLEEP
#define KS_WAKE KS_SYSTEM_WAKE

enum keysym_system {
	KS_SYSTEM_POWER = SPECIAL(0, S_SYSTEM),
	KS_SYSTEM_SLEEP,
	KS_SYSTEM_WAKE,
};

uint16_t keysym_to_system(uint32_t keysym);
