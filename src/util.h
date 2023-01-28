#pragma once

#include "ws2812.h"
#include "led.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))

#define ERR(...) stdio_log(LOG_ERR, "ERR  : " __VA_ARGS__)
#define WARN(...) stdio_log(LOG_WARN, "WARN : " __VA_ARGS__)
#define INFO(...) stdio_log(LOG_INFO, "INFO : " __VA_ARGS__)
#define DEBUG(...) stdio_log(LOG_DEBUG, "DEBUG: " __VA_ARGS__)

#define PANIC(...) blink_panic(200, HARD_RED, __VA_ARGS__);
#define ASSERT(cond) do { \
		if (!(cond)) PANIC("Assertion failed: (%s) in %s:%i", \
			#cond, __FILE__, __LINE__); \
	} while (0)

#define CLAIM_UNUSED_SM(pio) ({ \
	int tmp = pio_claim_unused_sm(pio, false); \
	ASSERT(tmp >= 0); \
	(uint) tmp; })

enum {
	LOG_NONE,
	LOG_ERR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
};

void stdio_log(int loglevel, const char *fmtstr, ...);

void blink_panic(uint32_t blink_ms, uint32_t rgb, const char *fmtstr, ...);

extern char warnlog[];
extern int loglevel;
