#ifndef BUTTONS_H
#define BUTTONS_H

#include <cstdint>

// Initialize button pins
void setupButtons();

// Handle button state transitions from readings
void updateButtonStates();

// Check if the button was just pressed
bool isButtonPressed(uint8_t pin);

// Check if the button is currently held
bool isButtonHeld(uint8_t pin);

// Check if the button was just released
bool isButtonReleased(uint8_t pin);

#endif // BUTTONS_H
