#include "board.h"
#include "class/cdc/cdc_device.h"
#include "class/hid/hid.h"
#include "device/usbd.h"
#include "neopix.h"
#include "util.h"

#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "pico/time.h"
#include "tusb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

enum {
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED = 1000,
	BLINK_SUSPENDED = 2500,
};

void stub_stdio_write(const char *buf, int len);
void stub_stdio_flush(void);
int stub_stdio_read(char *buf, int len);

bool send_hid_report(int id, bool state);

void hid_task(void);
void blink_task(void);

static struct stdio_driver usb_stdio = {
	.out_chars = stub_stdio_write,
	.out_flush = stub_stdio_flush,
	.in_chars = stub_stdio_read,
	.next = NULL
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static bool hit_state = false;

struct neopix onboard_led;

int
main(void)
{
	board_init();

	stdio_set_driver_enabled(&usb_stdio, true);
	stdio_init_all();

	neopix_init(&onboard_led, pio0, 0, 25);

	tud_init(BOARD_TUD_RHPORT);

	while (true) {
		tud_task();
		blink_task();
		hid_task();
	}

	return 0;
}

void
stub_stdio_write(const char *buf, int len)
{
	tud_cdc_write(buf, (uint32_t) len);
}

void
stub_stdio_flush(void)
{
	tud_cdc_write_flush();
}

int
stub_stdio_read(char *buf, int len)
{
	return (int) tud_cdc_read(buf, (uint32_t) len);
}

void
tud_mount_cb(void)
{
	blink_interval_ms = BLINK_MOUNTED;
}

void
tud_umount_cb(void)
{
	blink_interval_ms = BLINK_NOT_MOUNTED;
}

void
tud_suspend_cb(bool remote_wakeup_en)
{
	(void) remote_wakeup_en;
	blink_interval_ms = BLINK_SUSPENDED;
}

void
tud_resume_cb(void)
{
	blink_interval_ms = BLINK_MOUNTED;
}

void
tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	(void) itf;
	(void) rts;
	(void) dtr;
}

void
tud_cdc_rx_cb(uint8_t itf)
{
	(void) itf;
}

uint16_t
tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
	hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	(void) itf;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) reqlen;

	return 0;
}

void
tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
	hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	(void) itf;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) bufsize;
}

void
tud_hid_report_complete_cb(uint8_t instance,
	uint8_t const *report, uint8_t len)
{
	uint8_t id;

	(void) instance;
	(void) report;
	(void) len;

	for (id = report[0] + 1; id < REPORT_ID_MAX; id++) {
		if (send_hid_report(id, hit_state))
			break;
	}
}

bool
send_keyboard_report(bool state)
{
	static bool cleared = true;
	uint8_t keycode[6] = { 0 };

	if (state) {
		keycode[0] = HID_KEY_A;
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
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
send_hid_report(int id, bool state)
{
	switch (id) {
	case REPORT_ID_KEYBOARD:
		return send_keyboard_report(state);
	case REPORT_ID_MOUSE:
		return send_mouse_report(state);
	case REPORT_ID_CONSUMER_CONTROL:
		return send_consumer_control_report(state);
	}

	return false;
}

void
send_hid_report_timed(void)
{
	const uint32_t period_ms = 1000;
	static uint32_t start_ms = 0;

	if (!tud_hid_ready()) return;

	hit_state = (board_millis() - start_ms < period_ms);
	if (hit_state) start_ms += period_ms;

	send_hid_report(REPORT_ID_MIN, hit_state);
}

void
hid_task(void)
{
	const uint32_t poll_ms = 10;
	static uint32_t start_ms = 0;

	if (board_millis() - start_ms < poll_ms)
		return;
	start_ms += poll_ms;

	// if (tud_suspended()) {
	// 	tud_remote_wakeup();
	// 	return;
	// }

	send_hid_report_timed();
}

void
blink_task(void)
{
	static uint32_t start_ms = 0;
	static bool state = false;

	if (board_millis() - start_ms < blink_interval_ms)
		return;
	start_ms += blink_interval_ms;

	state ^= true;
	neopix_put(&onboard_led, neopix_u32rgb(255 * state, 0, 255 * state));

}
