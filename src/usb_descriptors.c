#include "hid.h"

#include "class/hid/hid_device.h"
#include "tusb.h"
#include "tusb_types.h"

#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))

#define CONFIG_TOTAL_LEN \
	(TUD_CONFIG_DESC_LEN + 2 * TUD_HID_DESC_LEN + TUD_CDC_DESC_LEN)

/* MCU-specific! */
#define EPNUM_HID_KBD    0x81
#define EPNUM_HID_MISC   0x82
#define EPNUM_CDC_NOTIF  0x84
#define EPNUM_CDC_IN     0x85
#define EPNUM_CDC_OUT    0x05

/* NOTE: same VID/PID with different interface can cause issues! */

enum {
	ITF_NUM_HID_KBD,
	ITF_NUM_HID_MISC,
	ITF_NUM_CDC,
	ITF_NUM_CDC_DATA,
	ITF_NUM_TOTAL
};

static const tusb_desc_device_t desc_device = {
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = 0x200,

	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor           = 0x1209,
	.idProduct          = 0xDEF0,
	.bcdDevice          = 0x0100,

	.iManufacturer      = 0x01,
	.iProduct           = 0x02,
	.iSerialNumber      = 0x03,

	.bNumConfigurations = 0x01
};

static const uint8_t desc_hid_kbd_report[] = {
	TUD_HID_REPORT_DESC_KEYBOARD()
};

static const uint8_t desc_hid_misc_report[] = {
	TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
	TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER)),
	TUD_HID_REPORT_DESC_SYSTEM_CONTROL(HID_REPORT_ID(REPORT_ID_SYSTEM)),
	TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD))
};

static const uint8_t desc_fs_configuration[] = {
	/* Config number, interface count, string index,
	 * total length, attribute, power in mA */
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
		TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

	/* Interface number, string index, protocol, report descriptor len,
	 * EP In address, size & polling interval */
	TUD_HID_DESCRIPTOR(ITF_NUM_HID_KBD, 5, HID_ITF_PROTOCOL_KEYBOARD,
		sizeof(desc_hid_kbd_report), EPNUM_HID_KBD, 8, 1),
	TUD_HID_DESCRIPTOR(ITF_NUM_HID_MISC, 6, HID_ITF_PROTOCOL_NONE,
		sizeof(desc_hid_misc_report), EPNUM_HID_MISC, 16, 1),

	/* Interface number, string index, EP notification address and size,
	 * EP data address (out, in) and size */
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8,
		EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

#if TUD_OPT_HIGH_SPEED
static const uint8_t desc_hs_configuration[] = {
	/* Config number, interface count, string index,
	 * total length, attribute, power in mA */
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
		TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

	/* Interface number, string index, protocol, report descriptor len,
	 * EP In address, size & polling interval */
	TUD_HID_DESCRIPTOR(ITF_NUM_HID_KBD, 5, HID_ITF_PROTOCOL_KEYBOARD,
		sizeof(desc_hid_kbd_report), EPNUM_HID_KBD, 8, 1),
	TUD_HID_DESCRIPTOR(ITF_NUM_HID_MISC, 6, HID_ITF_PROTOCOL_NONE,
		sizeof(desc_hid_misc_report), EPNUM_HID_MISC, 16, 1),

	/* Interface number, string index, EP notification address and size,
	 * EP data address (out, in) and size */
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8,
		EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
};

static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

static const tusb_desc_device_qualifier_t desc_device_qualifier =
{
	.bLength            = sizeof(tusb_desc_device_qualifier_t),
	.bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
	.bcdUSB             = 0x200,

	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	.bNumConfigurations = 0x01,
	.bReserved          = 0x00
};
#endif

static const char *string_desc_arr[] = {
	[0] = "\x07\x04",       /* LangID: German (0x0407) */
	[1] = "SNX",            /* Manufacturer */
	[2] = "SXKBD Keyboard", /* Product */
	[3] = "000001",         /* Serial Number */
	[4] = "CDC",
	[5] = "HID-KBD",
	[6] = "HID-MISC"
};

static uint16_t _desc_str[32];

const uint8_t *
tud_descriptor_device_cb(void)
{
	return (const uint8_t *) &desc_device;
}

const uint8_t *
tud_hid_descriptor_report_cb(uint8_t instance)
{
	if (instance == INST_HID_KBD)
		return desc_hid_kbd_report;
	else
		return desc_hid_misc_report;
}

const uint8_t *
tud_descriptor_configuration_cb(uint8_t instance)
{
#if TUD_OPT_HIGH_SPEED
	if (tud_speed_get() == TUSB_SPEED_HIGH)
		return desc_hs_configuration;
	else
		return desc_fs_configuration;
#else
	return desc_fs_configuration;
#endif
}

const uint16_t *
tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	const char *str;
	uint8_t i, chr_count;

	if (index == 0) {
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		chr_count = 1;
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
const uint8_t *
tud_descriptor_device_qualifier_cb(void)
{
	return (const uint8_t *) &desc_device_qualifier;
}

const uint8_t *
tud_descriptor_other_speed_configuration_cb(uint8_t instance)
{
	if (tud_speed_get() == TUSB_SPEED_HIGH) {
		memcpy(desc_other_speed_config, desc_hs_configuration,
			CONFIG_TOTAL_LEN);
	} else {
		memcpy(desc_other_speed_config, desc_fs_configuration,
			CONFIG_TOTAL_LEN);
	}

	desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

	return desc_other_speed_config;
}
#endif
