#include "keysym/system.h"

#include "hid/system.h"

#include <stdint.h>

uint16_t
keysym_to_system(uint32_t keysym)
{
	switch (keysym) {
		case KS_SYSTEM_POWER:
			return SYSTEM_POWER_DOWN;
		case KS_SYSTEM_SLEEP:
			return SYSTEM_SLEEP;
		case KS_SYSTEM_WAKE:
			return SYSTEM_WAKE_UP;
		default:
			return 0;
	}
}
