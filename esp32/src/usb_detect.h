#pragma once
#include <stdint.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

typedef enum {
    OS_UNKNOWN = 0,
    OS_MACOS,
    OS_LINUX,
    OS_WINDOWS,
} DetectedOS;

extern "C" uint16_t tud_descriptor_string_cb(uint8_t index, uint16_t langid);
extern "C" void tud_mount_cb(void);
extern "C" void tud_umount_cb(void);
DetectedOS wait_os_detection(void);
