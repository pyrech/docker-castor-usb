#include <Arduino.h>
#include "command.h"
#include "usb_detect.h"
#include "keyboard_utils.h"
#include "globals.h"

void open_terminal()
{
    if (hostOS == OS_WINDOWS) {
        Serial.println("Can not open a terminal: Windows is not supported");

        return;
    }

    if (hostOS == OS_MACOS) {
        // macOS → Spotlight
        Keyboard.press(KEY_LEFT_GUI);  // Command
        Keyboard.press(' ');           // Space
        Keyboard.releaseAll();
        delay(400);

        type_text("Terminal");
        delay(200);
        Keyboard.write(KEY_RETURN);

        // New window
        Keyboard.press(KEY_LEFT_GUI);  // Command
        Keyboard.press('n');           // N
        Keyboard.releaseAll();
    } else {
        // Linux (GNOME/KDE) → Ctrl+Alt+T
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press('t');
        Keyboard.releaseAll();
    }
 
    delay(600);
}

void send_command(const char *cmd)
{
    if (hostOS == OS_WINDOWS) {
        Serial.println("Can not send a command: Windows is not supported");

        return;
    }

    open_terminal();

    type_text(cmd);
    type_text("; exit");

    Keyboard.write(KEY_RETURN);
}
