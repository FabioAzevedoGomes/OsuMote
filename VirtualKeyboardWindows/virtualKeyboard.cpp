#include "virtualKeyboard.h"

INPUT createGenericKeyboard(INPUT keyboard) {
    // Set up a generic keyboard event.
    keyboard.type = INPUT_KEYBOARD;
    keyboard.ki.wScan = 0; // hardware scan code for key
    keyboard.ki.time = 0;
    keyboard.ki.dwExtraInfo = 0;

    return keyboard;
}

void pressKey(INPUT keyboard, int key) {
    keyboard.ki.wVk = key; // virtual-key code
    keyboard.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &keyboard, sizeof(INPUT));
}

void releaseKey(INPUT keyboard) {
    keyboard.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
    SendInput(1, &keyboard, sizeof(INPUT));
}

void extern keystroke(int key) {
    
    // Create virtual keyboard
    INPUT keyboard;
    keyboard = createGenericKeyboard(keyboard);
    
    // Press the desired key
    pressKey(keyboard, key);
    releaseKey(keyboard);
}
