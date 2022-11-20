#pragma once

#include <stdbool.h>
#include <stdarg.h>

#define ERR(...) stdio_log(LOG_ERR, __VA_ARGS__)
#define WARN(...) stdio_log(LOG_WARN, __VA_ARGS__)
#define INFO(...) stdio_log(LOG_INFO, __VA_ARGS__)
#define DEBUG(...) stdio_log(LOG_DEBUG, __VA_ARGS__)
#define PANIC(...) blink_panic(__VA_ARGS__);
#define ASSERT(cond) blink_panic("Assertion failed: (%s) in %s:%i", \
	#cond, __FILE__, __LINE__)

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
