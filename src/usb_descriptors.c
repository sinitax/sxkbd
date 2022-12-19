#include "hid.h"

#include "class/hid/hid_device.h"
#include "tusb.h"
#include "tusb_types.h"

#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))

/* same VID/PID with different interface can cause issues! */

#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) \
	 | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define USB_VID 0xC0FF
#define USB_BCD 0x0200

#define CONFIG_TOTAL_LEN \
	(TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)

/* MCU-specific! */
#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82
#define EPNUM_HID         0x83

enum {
	ITF_NUM_CDC,
	ITF_NUM_CDC_DATA,
	ITF_NUM_HID,
	ITF_NUM_TOTAL
};

tusb_desc_device_t const desc_device = {
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = USB_BCD,

	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0100,

	.iManufacturer      = 0x01,
	.iProduct           = 0x02,
	.iSerialNumber      = 0x03,

	.bNumConfigurations = 0x01
};

uint8_t const desc_hid_report[] = {
	TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
	TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
	TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL))
};

uint8_t const desc_fs_configuration[] = {
	/* Config number, interface count, string index,
	 * total length, attribute, power in mA */
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
		TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

	/* Interface number, string index, EP notification address and size,
	 * EP data address (out, in) and size */
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8,
		EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

	/* Interface number, string index, protocol, report descriptor len,
	 * EP In address, size & polling interval */
	TUD_HID_DESCRIPTOR(ITF_NUM_HID, 5, HID_ITF_PROTOCOL_NONE,
		sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1)
};

#if TUD_OPT_HIGH_SPEED
uint8_t const desc_hs_configuration[] = {
	/* Config number, interface count, string index,
	 * total length, attribute, power in mA */
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
		TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

	/* Interface number, string index, EP notification address and size,
	 * EP data address (out, in) and size */
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8,
		EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),

	/* Interface number, string index, protocol, report descriptor len,
	 * EP In address, size & polling interval */
	TUD_HID_DESCRIPTOR(ITF_NUM_HID, 5, HID_ITF_PROTOCOL_NONE,
		sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

tusb_desc_device_qualifier_t const desc_device_qualifier =
{
	.bLength            = sizeof(tusb_desc_device_qualifier_t),
	.bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
	.bcdUSB             = USB_BCD,

	.bDeviceClass       = TUSB_CLASS_MISC,
	.bDeviceSubClass    = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol    = MISC_PROTOCOL_IAD,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	.bNumConfigurations = 0x01,
	.bReserved          = 0x00
};
#endif

char const *string_desc_arr[] = {
	[0] = "\x09\x04\x09\x00", /* bCountryCode: Germany */
	[1] = "TinyUSB",          /* Manufacturer */
	[2] = "TinyUSB Device",   /* Product */
	[3] = "123456",           /* Serials, should use chip ID */
	[4] = "SXKBD CDC",
	[5] = "SXKBD HID"
};

static uint16_t _desc_str[32];

/* Invoked on GET DEVICE DESCRIPTOR */
uint8_t const *
tud_descriptor_device_cb(void)
{
	return (uint8_t const *) &desc_device;
}

/* Invoked on GET HID REPORT DESCRIPTOR */
uint8_t const *
tud_hid_descriptor_report_cb(uint8_t itf)
{
	(void) itf;
	return desc_hid_report;
}

/* Invoked on GET CONFIGURATION DESCRIPTOR */
uint8_t const *
tud_descriptor_configuration_cb(uint8_t index)
{
	(void) index;

#if TUD_OPT_HIGH_SPEED
	return (tud_speed_get() == TUSB_SPEED_HIGH) ?
		desc_hs_configuration : desc_fs_configuration;
#else
	return desc_fs_configuration;
#endif
}

/* Invoked on GET STRING DESCRIPTOR */
uint16_t const *
tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	const char *str;
	uint8_t i, chr_count;

	(void) langid;

	if (index == 0) {
		memcpy(&_desc_str[1], string_desc_arr[0], 4);
		chr_count = 2;
	} else {
		if (index >= ARRLEN(string_desc_arr))
			return NULL;

		str = string_desc_arr[index];

		chr_count = (uint8_t) strlen(str);
		if (chr_count > 31) chr_count = 31;

		/* Convert ASCII string into UTF-16 */
		for (i = 0; i < chr_count; i++)
			_desc_str[i+1] = str[i];
	}

	/* first byte is length (including header), second byte is type */
	_desc_str[0] = (uint8_t) (2 * chr_count + 2);
	_desc_str[0] |= (uint16_t) (TUSB_DESC_STRING << 8);

	return _desc_str;
}

#if TUD_OPT_HIGH_SPEED
uint8_t const *
tud_descriptor_device_qualifier_cb(void)
{
	return (uint8_t const*) &desc_device_qualifier;
}

/* Invoked on GET OTHER SEED CONFIGURATION DESCRIPTOR */
uint8_t const *
tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
	(void) index;

	memcpy(desc_other_speed_config, (tud_speed_get() == TUSB_SPEED_HIGH)
		? desc_fs_configuration : desc_hs_configuration,
		CONFIG_TOTAL_LEN);

	desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

	return desc_other_speed_config;
}
#endif
