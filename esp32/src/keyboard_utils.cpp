#include "keyboard_utils.h"
#include "globals.h"
#include "usb_detect.h"

char azerty_to_qwerty(char c) {
    switch (c) {
        // Letters (AZERTY inversions)
        case 'a': return 'q';
        case 'A': return 'Q';
        case 'z': return 'w';
        case 'Z': return 'W';
        case 'q': return 'a';
        case 'Q': return 'A';
        case 'w': return 'z';
        case 'W': return 'Z';

        // M is on the ; physical key
        case 'm': return ';'; 
        case 'M': return ':';

        // Numbers (AZERTY = Shift + Key)
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';

        case '/': return '>'; 
        case '.': return '<'; 
        case ':': return '.'; 
        case ';': return ',';
        case ',': return 'm';
        case '?': return 'M'; // Shift + , = ?
        case '!': return '/';
        case '&': return '1';
        case '"': return '3';
        case '\'': return '4';
        case '(': return '5';
        case '-': return hostOS == OS_MACOS ? 0 : '6';
        case '_': return hostOS == OS_MACOS ? '_' : '8';
        case '+': return hostOS == OS_MACOS ? 0 : '+'; // Shift + =
        case ')': return '-';
        case '$': return ']';

        // Not handled chars (dead keys, AltGr)
        case '~': return 0;
        case '|': return 0;
        case '[': return 0;
        case ']': return 0;
        case '\\': return 0;

        // By default, send the same character (b, c, d, e, f...)
        default: return c;
    }
}

void type_text(const char* text) {
    Serial.println("Writing to keyboard:");
    Serial.println(text);

    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        char input = text[i];
        char output = azerty_to_qwerty(input);

        if (output != 0) {
            // Simple case : direct translation possible
            Keyboard.write(output);
        } else {
            // Complex cases (AltGr / Dead keys)
            if (input == '~') {
                if (hostOS == OS_MACOS) {
                    // Mac: Alt(Option) + N, then Space
                    Keyboard.press(KEY_LEFT_ALT); Keyboard.write('n'); Keyboard.releaseAll();
                    Keyboard.write(' ');
                } else {
                    // Linux: AltGr + 2 (US '2' is physical 'é/~')
                    Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('2'); Keyboard.releaseAll();
                }
            }
            else if (input == '|') {
                if (hostOS == OS_MACOS) {
                    // Mac: Shift + Alt + L
                    Keyboard.press(KEY_LEFT_SHIFT); Keyboard.press(KEY_LEFT_ALT); Keyboard.write('l'); Keyboard.releaseAll();
                } else {
                    // Linux: AltGr + 6 (US '6' is physical '-/|')
                    Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('6'); Keyboard.releaseAll();
                }
            }
            else if (input == '[') {
                if (hostOS == OS_MACOS) {
                    // Mac: Shift + Alt + 5
                    Keyboard.press(KEY_LEFT_SHIFT); Keyboard.press(KEY_LEFT_ALT); Keyboard.write('5'); Keyboard.releaseAll();
                } else {
                    // Linux: AltGr + 5
                    Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('5'); Keyboard.releaseAll();
                }
            }
            else if (input == ']') {
                if (hostOS == OS_MACOS) {
                    // Mac: Shift + Alt + ) (US '-')
                    Keyboard.press(KEY_LEFT_SHIFT); Keyboard.press(KEY_LEFT_ALT); Keyboard.write('-'); Keyboard.releaseAll();
                } else {
                    // Linux: AltGr + ) (US '-')
                    Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('-'); Keyboard.releaseAll();
                }
            }
            else if (input == '+') {
                // Code 223 = Keypad +
                Keyboard.press(223); delay(2); Keyboard.releaseAll();
            }
            else if (input == '-') {
                // Code 222 = Keypad Minus
                Keyboard.press(222); delay(2); Keyboard.releaseAll();
            }
        }
        delay(2);
    }
}
