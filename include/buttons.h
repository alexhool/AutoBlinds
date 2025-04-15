#ifndef BUTTONS_H
#define BUTTONS_H

// Button pins
#define BTN_OPEN 12
#define BTN_CLOSE 13
#define BTN_MODE 19

// Button constants
#define BTN_DEBOUNCE 50
#define HOLD_TIME 3000

// Initialize button pins
void buttonsInit();

// Update button states
void updateButtons();

// Process button events
void handleButtons();

// Get current button states
bool getDebouncedOpen();
bool getDebouncedClose();
bool getDebouncedMode();

#endif // BUTTONS_H