#include "usb_stdio.h"

#include "pico/stdio/driver.h"
#include "tusb.h"

static void usb_stdio_write(const char *buf, int len);
static void usb_stdio_flush(void);
static int usb_stdio_read(char *buf, int len);

static struct stdio_driver usb_stdio = {
	.out_chars = usb_stdio_write,
	.out_flush = usb_stdio_flush,
	.in_chars = usb_stdio_read,
	.next = NULL
};

void
usb_stdio_write(const char *buf, int len)
{
	tud_cdc_write(buf, (uint32_t) len);
}

void
usb_stdio_flush(void)
{
	tud_cdc_write_flush();
}

int
usb_stdio_read(char *buf, int len)
{
	return (int) tud_cdc_read(buf, (uint32_t) len);
}

void
usb_stdio_init(void)
{
	stdio_set_driver_enabled(&usb_stdio, true);
	stdio_init_all();
}
