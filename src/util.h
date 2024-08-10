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
	LOG_NONE   = 0b000000,
	LOG_MISC   = 0b000001,
	LOG_KEYMAT = 0b000010,
	LOG_KEYMAP = 0b000100,
	LOG_HID    = 0b001000,
	LOG_TIMING = 0b010000,
	LOG_SPLIT  = 0b100000,
	LOG_ALL    = 0b111111,
};

void stdio_log(int group, int level, const char *fmtstr, ...);

void blink_panic(uint32_t blink_ms, uint32_t rgb, const char *fmtstr, ...);

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

extern char warnlog[];
extern int log_level_min;
extern int log_group_mask;
