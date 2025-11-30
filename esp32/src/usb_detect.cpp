#include <Arduino.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "esp_timer.h"
#include "usb_detect.h"

volatile DetectedOS hostOS = OS_UNKNOWN;
volatile bool usb_is_mounted = false;
volatile bool macos_fingerprint_check = false;
static int64_t detect_start_time_us = 0;

extern "C" uint16_t tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    if (index == 0xEE) {
        hostOS = OS_WINDOWS;
    } else if (index == 0 && hostOS != OS_WINDOWS) {
        // Trigger macOS/Linux flag
        macos_fingerprint_check = true; 
    }

    // Let TinyUSB's native stack handle the response for every descriptors
    return 0;
}

// Callback function for USB mount event
extern "C" void tud_mount_cb(void) {
    usb_is_mounted = true;

    if (hostOS != OS_WINDOWS) {
        if (macos_fingerprint_check) {
            // If Index 0 request was triggered, it's probably Linux
            hostOS = OS_LINUX;
        } else {
            // If no 0xEE and Index 0 was not asked - or late or in non standard ways, it's probably macOS
            // macOS is often more discrete/complexe before mount
            hostOS = OS_MACOS;
        }
    }
}

// Call when USB is unmount of host was reset
extern "C" void tud_umount_cb(void) {
    usb_is_mounted = false;
    hostOS = OS_UNKNOWN; 
    macos_fingerprint_check = false;
}

DetectedOS wait_os_detection()
{
    detect_start_time_us = esp_timer_get_time();

    // Wait for 1.5 secondes to watch for traffic
    while (esp_timer_get_time() - detect_start_time_us < 1500000) {
        if (usb_is_mounted) {
            break;
        }
        delay(10);
    }

    return hostOS;
}
