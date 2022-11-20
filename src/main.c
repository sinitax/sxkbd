#include "board.h"
#include "class/cdc/cdc_device.h"
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

void led_blinking_task(void);

static struct stdio_driver usb_stdio = {
	.out_chars = stub_stdio_write,
	.out_flush = stub_stdio_flush,
	.in_chars = stub_stdio_read,
	.next = NULL
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

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
		led_blinking_task();
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

/* Invoked on GET_REPORT */
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

/* Invoked on SET_REPORT or receive on OUT with (Report ID = 0, Type = 0) */
void
tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
	hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	(void) itf;
	(void) report_id;
	(void) report_type;

	tud_hid_report(0, buffer, bufsize);
}

void
led_blinking_task(void)
{
	static uint32_t start_ms = 0;
	static bool state = false;

	if (board_millis() - start_ms < blink_interval_ms)
		return;

	neopix_put(&onboard_led, neopix_u32rgb(255 * state, 0, 255 * state));

	start_ms += blink_interval_ms;
	state ^= true;
}
