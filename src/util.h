#pragma once

#include <stdbool.h>
#include <stdarg.h>

#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))

#define ERR(...) stdio_log(LOG_ERR, "ERR  : " __VA_ARGS__)
#define WARN(...) stdio_log(LOG_WARN, "WARN : " __VA_ARGS__)
#define INFO(...) stdio_log(LOG_INFO, "INFO : " __VA_ARGS__)
#define DEBUG(...) stdio_log(LOG_DEBUG, "DEBUG: " __VA_ARGS__)

#define PANIC(...) blink_panic(__VA_ARGS__);
#define ASSERT(cond) do { \
		if (!(cond)) blink_panic("Assertion failed: (%s) in %s:%i", \
			#cond, __FILE__, __LINE__); \
	} while (0)

enum {
	LOG_NONE,
	LOG_ERR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
};

void stdio_log(int loglevel, const char *fmtstr, ...);

void blink_panic(const char *fmtstr, ...);

extern int loglevel;
