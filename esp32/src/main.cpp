#include <Arduino.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include <Bounce2.h>

#include "usb_detect.h"
#include "command.h"
#include "globals.h"

USBHIDKeyboard Keyboard;
DetectedOS os;

#define BUTTON_STOP 1
#define BUTTON_START 2

Bounce debStop = Bounce();
Bounce debStart = Bounce();

// Timing constants
const unsigned long DOUBLE_CLICK_DELAY = 350;
const unsigned long COMBO_WINDOW = 200;
const unsigned long COMBO_LONG_HOLD = 2000;

// Internal state
bool pendingStart = false;
bool pendingStop  = false;

unsigned long startClickTime = 0;
unsigned long stopClickTime  = 0;

bool comboCandidate = false;
unsigned long comboStartTime = 0;

void singleClickStop()
{
    send_command("docker-castor-usb stop");
}

void singleClickStart()
{
    send_command("docker-castor-usb start");
}

void doubleClickStart()
{
    send_command("docker-castor-usb previous");
}

void bothClick()
{
    send_command("curl -fsSL https://raw.githubusercontent.com/pyrech/docker-castor-usb/main/castor.php -o ~/.local/bin/docker-castor-usb && chmod +x ~/.local/bin/docker-castor-usb");
}

void setup()
{
    Serial.begin(115200);
    Keyboard.begin();
    USB.begin();

    delay(2000); 
    Serial.println("USB Serial initialized!");

    pinMode(BUTTON_STOP, INPUT_PULLUP);
    pinMode(BUTTON_START, INPUT_PULLUP);

    debStop.attach(BUTTON_STOP);
    debStop.interval(10);

    debStart.attach(BUTTON_START);
    debStart.interval(10);

    Serial.println("Waiting OS detection...");
    os = wait_os_detection();

    Serial.print("OS detected: ");
    if (os == OS_MACOS) Serial.println("macOS");
    else if (os == OS_LINUX) Serial.println("Linux");
    else if (os == OS_WINDOWS) Serial.println("Windows");
    else Serial.println("Unknown");

    delay(1000);

    Serial.println("Downloading castor binary");

    send_command("test -f ~/.local/bin/docker-castor-usb || curl -fsSL https://raw.githubusercontent.com/pyrech/docker-castor-usb/main/castor.php -o ~/.local/bin/docker-castor-usb && chmod +x ~/.local/bin/docker-castor-usb");
}

void loop()
{
    debStop.update();
    debStart.update();

    unsigned long now = millis();

    // Detect button FALL events (button pressed)
    if (debStart.fell()) {
        pendingStart = true;
        startClickTime = now;
    }

    if (debStop.fell()) {
        pendingStop = true;
        stopClickTime = now;
    }

    // Detect combo candidate (both buttons pressed within window)
    if (!comboCandidate && pendingStart && pendingStop) {
        if (abs((long)startClickTime - (long)stopClickTime) <= COMBO_WINDOW) {
            comboCandidate = true;
            comboStartTime = now;
            Serial.println("Combo candidate detected");
        }
    }

    // Handle COMBO long press
    if (comboCandidate) {
        // If either button is released, cancel combo
        if (debStart.rose() || debStop.rose()) {
            comboCandidate = false;
            pendingStart = pendingStop = false;
            return;
        }

        // Long hold reached
        if (now - comboStartTime >= COMBO_LONG_HOLD) {
            Serial.println("Combo long-press triggered!");
            bothClick();
            comboCandidate = false;
            pendingStart = pendingStop = false;
            return;
        }

        return; // During combo wait, ignore other logic
    }

    // Handle STOP single-click (no double-click on STOP)
    if (pendingStop && (now - stopClickTime > COMBO_WINDOW)) {
        // Only execute if START wasn't pressed during window
        if (!pendingStart) {
            Serial.println("STOP single click");
            singleClickStop();
            pendingStop = false;
        }
    }

    // Handle START single-click / double-click
    if (pendingStart) {
        // Double click check
        if (debStart.fell() && (now - startClickTime <= DOUBLE_CLICK_DELAY)) {
            Serial.println("START double click");
            doubleClickStart();
            pendingStart = false;
            return;
        }

        // Single click (after delay and no double)
        if (now - startClickTime > DOUBLE_CLICK_DELAY) {
            Serial.println("START single click");
            singleClickStart();
            pendingStart = false;
        }
    }
}
