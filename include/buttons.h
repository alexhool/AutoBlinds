#ifndef BUTTONS_H
#define BUTTONS_H

#include <cstdint>

// Button states
enum class ButtonInputState {
  IDLE,      // Default
  PRESSED,
  HELD,
  RELEASED
};

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
