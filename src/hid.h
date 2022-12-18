#pragma once

#include <stdbool.h>
#include <stdint.h>

#define REPORT_ID_MIN REPORT_ID_KEYBOARD
enum {
	REPORT_ID_KEYBOARD = 1,
	REPORT_ID_MOUSE,
	REPORT_ID_CONSUMER_CONTROL,
	REPORT_ID_MAX
};

void hid_init(void);
void hid_task(void);
