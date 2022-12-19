#pragma once

/* Generic Desktop Page (0x01)
 *
 * See https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf#page=26
 */
enum hid_system {
    /* 4.5.1 System Controls - Power Controls */
    SYSTEM_POWER_DOWN             = 0x81,
    SYSTEM_SLEEP                  = 0x82,
    SYSTEM_WAKE_UP                = 0x83,
    SYSTEM_RESTART                = 0x8F,

    /* 4.10 System Display Controls */
    SYSTEM_DISPLAY_TOGGLE_INT_EXT = 0xB5
};

