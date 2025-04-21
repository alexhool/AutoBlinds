#include "buttons.h"
#include <Arduino.h>
#include "config.h"

// Button states
enum class ButtonState {
  IDLE,      // Default
  PRESSED,
  HELD,
  RELEASED
};

// Button state information
struct ButtonInfo {
  const uint8_t pin;              // GPIO pin number
  ButtonState state;              // Current debounced state
  bool lastReading;               // Last raw reading
  unsigned long lastStateTime;    // Time of last state change
  unsigned long pressStartTime;   // Time of initial press
  bool holdTriggered;             // Flag indicating if button is held
};

// Button state array with initial values
static ButtonInfo buttons[] = {
  {PIN_BTN_OPEN, ButtonState::IDLE, LOW, 0, 0, false},
  {PIN_BTN_CLOSE, ButtonState::IDLE, LOW, 0, 0, false},
  {PIN_BTN_MODE, ButtonState::IDLE, LOW, 0, 0, false}
};
static constexpr uint8_t numButtons = sizeof(buttons) / sizeof(ButtonInfo);

// Initialize button GPIO with internal pull-down resistors
void setupButtons() {
  Serial.print("Initializing Buttons...");

  for (int i = 0; i < numButtons; ++i) {
    pinMode(buttons[i].pin, INPUT_PULLDOWN);
  }

  Serial.print("Done\n");
}

// Handle button state transitions from debounced readings
void updateButtonStates() {
  unsigned long currentTime = millis();

  for (int i = 0; i < numButtons; ++i) {
    bool reading = digitalRead(buttons[i].pin);

    // Debounce button reading
    if (reading != buttons[i].lastReading) {
      buttons[i].lastStateTime = currentTime;
    }
    if ((currentTime - buttons[i].lastStateTime) > BTN_DEBOUNCE) {
      bool isPressed = (reading == HIGH);

      // Compare debounced reading with current state
      if (isPressed != (buttons[i].state == ButtonState::PRESSED ||
          buttons[i].state == ButtonState::HELD)) {
        // Button state changed from IDLE/RELEASED
        if (isPressed) {
          buttons[i].state = ButtonState::PRESSED;
          buttons[i].pressStartTime = currentTime;
          buttons[i].holdTriggered = false;
        } else {
          buttons[i].state = ButtonState::RELEASED;
        }
      }
      // Check if the button is being held
      else if (buttons[i].state == ButtonState::PRESSED && !buttons[i].holdTriggered) {
        // Determine hold duration based on button type (open/close default to 500ms)
        unsigned long holdDuration = (buttons[i].pin == PIN_BTN_MODE) ? CONFIG_HOLD_TIME : 500;

        // Check if the hold duration has been met
        if ((currentTime - buttons[i].pressStartTime) >= holdDuration) {
          buttons[i].state = ButtonState::HELD;
          buttons[i].holdTriggered = true;
        }
      }
    }

    // Store current reading for next call
    buttons[i].lastReading = reading;
  }
}

// Check if the button was just pressed
bool isButtonPressed(uint8_t pin) {
  for (int i = 0; i < numButtons; ++i) {
    if (buttons[i].pin == pin) {
      return buttons[i].state == ButtonState::PRESSED;
    }
  }
  return false;
}

// Check if the button is currently held
bool isButtonHeld(uint8_t pin) {
  for (int i = 0; i < numButtons; ++i) {
    if (buttons[i].pin == pin) {
      return buttons[i].state == ButtonState::HELD;
    }
  }
  return false;
}

// Check if the button was just released
bool isButtonReleased(uint8_t pin) {
  for (int i = 0; i < numButtons; ++i) {
    if (buttons[i].pin == pin) {
      if (buttons[i].state == ButtonState::RELEASED) {
        // Transition RELEASED state to IDLE
        buttons[i].state = ButtonState::IDLE;
        return true;
      }
      break;
    }
  }
  return false;
}
