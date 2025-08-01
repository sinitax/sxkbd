#pragma once

#include "ws2812.h"
#include "led.h"

#include "pico/time.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>

#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))

#define WARN(group, ...) stdio_log(group, LOG_WARN, "WARN : " __VA_ARGS__)
#define INFO(group, ...) stdio_log(group, LOG_INFO, "INFO : " __VA_ARGS__)
#define DEBUG(group, ...) stdio_log(group, LOG_DEBUG, "DEBUG: " __VA_ARGS__)

#define PANIC(...) blink_panic(200, HARD_RED, __VA_ARGS__);
#define ASSERT(cond) do { \
		if (!(cond)) PANIC("Assertion failed: (%s) in %s:%i", \
			#cond, __FILE__, __LINE__); \
	} while (0)

enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN
};

enum {
	LOG_NONE   = 0b0000000,
	LOG_MISC   = 0b0000001,
	LOG_KEYMAT = 0b0000010,
	LOG_KEYMAP = 0b0000100,
	LOG_HID    = 0b0001000,
	LOG_TIMING = 0b0010000,
	LOG_SPLIT  = 0b0100000,
	LOG_SLAVE  = 0b1000000,
	LOG_ALL    = 0b1111111,
};

void stdio_log(int group, int level, const char *fmtstr, ...);

void blink_panic(uint32_t blink_ms, uint32_t rgb, const char *fmtstr, ...);

void tud_sleep_ms(uint32_t millis);

static inline uint
claim_unused_sm(PIO pio)
{
	int tmp = pio_claim_unused_sm(pio, false);
	ASSERT(tmp >= 0);
	return (uint) tmp;
}

static inline uint64_t
board_micros(void)
{
	return to_us_since_boot(get_absolute_time());
}

extern char warnlog[256];
extern int log_level_min;
extern int log_group_mask;
