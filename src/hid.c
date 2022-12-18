#include "hid.h"
#include "split.h"
#include "keymat.h"
#include "keysym.h"
#include "keymap.h"

#include "bsp/board.h"
#include "pico/types.h"

void
hid_init(void)
{
	
}

bool
hid_gen_report(uint8_t *report)
{
	int keycnt;
	uint x, y, p;

	keycnt = 0;
	for (y = 0; y < KEY_ROWS * 2; y++) {
		for (x = 0; x < KEY_COLS; x++) {
			if (!keymat[y][x])
				continue;
			if (keycnt >= 6) break;
			DEBUG("PRESS %i %", x, y);
			p = y * KEY_COLS + x;
			report[keycnt] = TO_CODE(keymap_layers[0][p]);
			keycnt++;
		}
	}

	return keycnt > 0;
}

bool
send_keyboard_report(void)
{
	static bool cleared = true;
	uint8_t report[6] = { 0 };
	bool any;

	any = hid_gen_report(report);

	if (any) {
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, report);
		cleared = false;
		return true;
	} else if (!cleared) {
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
		cleared = true;
		return true;
	}

	return false;
}

bool
send_mouse_report(bool state)
{
	if (state) {
		tud_hid_mouse_report(REPORT_ID_MOUSE, 0, 10, 10, 0, 0);
		return true;
	}

	return false;
}

bool
send_consumer_control_report(bool state)
{
	static bool cleared = true;
	uint16_t report;

	if (state) {
		report = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
		tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report, 2);
		cleared = false;
		return true;
	} else if (!cleared) {
		report = 0;
		tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report, 2);
		cleared = true;
		return true;
	}

	return false;
}

bool
send_hid_report(int id)
{
	if (!tud_hid_ready()) return false;

	switch (id) {
	case REPORT_ID_KEYBOARD:
		return send_keyboard_report();
	case REPORT_ID_MOUSE:
		return send_mouse_report(false);
	case REPORT_ID_CONSUMER_CONTROL:
		return send_consumer_control_report(false);
	}

	return false;
}

void
tud_hid_report_complete_cb(uint8_t instance,
	uint8_t const *report, uint8_t len)
{
	uint8_t id;

	for (id = report[0] + 1; id < REPORT_ID_MAX; id++) {
		if (send_hid_report(id))
			break;
	}
}

void
hid_task(void)
{
	send_hid_report(REPORT_ID_MIN);
}
