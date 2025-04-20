#include "states.h"
#include <Arduino.h>
#include <ESP32PCNTEncoder.h>
#include "config.h"
#include "memory.h"
#include "buttons.h"
#include "motor.h"
#include "tof.h"

// RGB LED color definitions
#define RGB_RED 255, 0, 0
#define RGB_GREEN 0, 255, 0
#define RGB_YELLOW 255, 255, 0
#define RGB_CYAN 0, 255, 255
#define RGB_OFF 0, 0, 0

// System operational states
enum class SystemState {
  TOGGLE_IDLE,    // 0 - Default
  TOGGLE_OPEN,    // 1
  TOGGLE_CLOSE,   // 2
  MANUAL_IDLE,    // 3
  MANUAL_MOVE,    // 4
  CONFIG_OPEN,    // 5
  CONFIG_CLOSE,   // 6
  CONFIG_SAVE,    // 7
  ERROR           // 8
};

// LED status states
enum LEDStatus {
  STATUS_TOGGLE_IDLE,   // Default
  STATUS_TOGGLE_OPEN,
  STATUS_TOGGLE_CLOSE,
  STATUS_MANUAL,
  STATUS_CONFIG_OPEN,
  STATUS_CONFIG_CLOSE,
  STATUS_CONFIG_SAVE,
  STATUS_ERROR
};

// Get global encoder object from motor.h
extern ESP32PCNTEncoder encoder;

// State machine variables
static SystemState currentState = SystemState::TOGGLE_IDLE;
static SystemState previousState = SystemState::TOGGLE_IDLE;

// Position control variables
static int64_t openPos = 0;
static int64_t closePos = 0;
static int64_t targetPos = 0;
static int64_t tempOpenPos = 0;
static int64_t tempClosePos = 0;
static unsigned long lastActivityTime = 0;

// Button state variables
static bool ignoreModeRelease = false;
static bool ignoreOpenRelease = false;
static bool ignoreCloseRelease = false;

// LED control variables
static LEDStatus currentLEDStatus = STATUS_TOGGLE_IDLE;
static unsigned long statusStartTime = 0;

// Forward declarations
static void enterState(SystemState newState);
static void handleToggleModeIdle();
static void handleToggleModeMoving();
static void handleManualMode();
static void handleConfigSetting();
static void handleConfigModeSaving();
static void updateLedIndicator(SystemState systemState);

// Initialize state machine to initial values
void setupStates() {
  Serial.print("Initializing States...");

  loadPositions(openPos, closePos);
  enterState(SystemState::TOGGLE_IDLE);

  Serial.println("Done");
  Serial.printf("(Loaded positions: Open = %lld, Close = %lld)\n", openPos, closePos);
}

// Handle system state transitions and logic
void updateStateMachine() {
  // Fetch new button states
  updateButtonStates();

  // Execute state-specific logic
  switch (currentState) {
    case SystemState::TOGGLE_IDLE:
      handleToggleModeIdle();
      break;
    case SystemState::TOGGLE_OPEN:
    case SystemState::TOGGLE_CLOSE:
      handleToggleModeMoving();
      break;
    case SystemState::MANUAL_IDLE:
    case SystemState::MANUAL_MOVE:
      handleManualMode();
      break;
    case SystemState::CONFIG_OPEN:
    case SystemState::CONFIG_CLOSE:
      handleConfigSetting();
      break;
    case SystemState::CONFIG_SAVE:
      handleConfigModeSaving();
      break;
    case SystemState::ERROR:
    default:
      break;
  }

  // Update LED based on current state after handling
  updateLedIndicator(currentState); // maybe move before switch?
}

// Transition to a new state and update LED
static void enterState(SystemState newState) {
  if (newState != currentState) {
    Serial.printf("\nState Change: %d -> %d\n", (int)currentState, (int)newState);
    previousState = currentState;
    currentState = newState;
    lastActivityTime = millis();

    // Execute state-specific logic
    switch (newState) {
      case SystemState::TOGGLE_IDLE:
      case SystemState::MANUAL_IDLE:
        motorStop();
        break;
      case SystemState::CONFIG_OPEN:
        Serial.println("Set OPEN Limit");
        motorStop();
        tempOpenPos = 0;
        tempClosePos = 0;
        ignoreModeRelease = true;
        break;
      case SystemState::CONFIG_CLOSE:
        Serial.println("Set CLOSE Limit");
        motorStop();
        break;
      case SystemState::CONFIG_SAVE:
        motorStop();
        break;
      case SystemState::ERROR:
        Serial.println("ERROR");
        motorStop();
        break;
      default:
        break;
    }

    // Update LED indicator based on new state
    updateLedIndicator(newState);
  }
}

// Move motor to new target position
static void startMovingTo(int64_t newTarget) {
  int64_t currentPos = encoder.getPosition();
  targetPos = newTarget;

  // Check if already at target
  if (abs(currentPos - targetPos) <= POS_TOLERANCE) {
    Serial.println("Already at target position");
    if (currentState == SystemState::TOGGLE_OPEN || currentState == SystemState::TOGGLE_CLOSE) {
      enterState(SystemState::TOGGLE_IDLE);
    }
    return;
  }

  int motorSpeed = 0;
  SystemState nextState = currentState;
  // Determine motor direction
  if (targetPos > currentPos) {
    motorSpeed = MOTOR_DEFAULT_SPEED;
    Serial.printf("Moving to %lld (current: %lld)\n", targetPos, currentPos);
  } else {
    motorSpeed = -MOTOR_DEFAULT_SPEED;
    Serial.printf("Moving to %lld (current: %lld)\n", targetPos, currentPos);
  }

  // Determine next state based on target position
  if (newTarget == openPos && currentState == SystemState::TOGGLE_IDLE) {
    nextState = SystemState::TOGGLE_OPEN;
  } else if (newTarget == closePos && currentState == SystemState::TOGGLE_IDLE) {
    nextState = SystemState::TOGGLE_CLOSE;
  } else {
    Serial.printf("ERROR: startMovingTo(%lld) from state %d\n", newTarget, (int)currentState);
    motorStop();
    enterState(SystemState::ERROR);
    return;
  }

  // Move motor
  motorMove(motorSpeed);
  if (nextState != currentState) {
    enterState(nextState);
  }
  lastActivityTime = millis();
}

// Handle motor movement for Manual/Config mode
static bool handleMotorMovement(int speed) {
  unsigned long currentTime = millis();
  bool openHeld = isButtonHeld(PIN_BTN_OPEN);
  bool closeHeld = isButtonHeld(PIN_BTN_CLOSE);
  bool moving = false;

  // Postive direction
  if (openHeld && !closeHeld) {
    motorMove(speed);
    lastActivityTime = currentTime;
    moving = true;
  }
  // Negative direction
  else if (closeHeld && !openHeld) {
    motorMove(-speed);
    lastActivityTime = currentTime;
    moving = true;
  }
  // Stop movement
  else {
    motorStop();
    if (isButtonPressed(PIN_BTN_OPEN) || isButtonReleased(PIN_BTN_OPEN) ||
        isButtonPressed(PIN_BTN_CLOSE) || isButtonReleased(PIN_BTN_CLOSE)) {
      lastActivityTime = currentTime;
    }
    moving = false;
  }

  return moving;
}

// Handle logic for TOGGLE_IDLE state
static void handleToggleModeIdle() {
  // Check for mode change
  if (isButtonHeld(PIN_BTN_MODE)) {
    enterState(SystemState::CONFIG_OPEN);
    return;
  }
  if (isButtonReleased(PIN_BTN_MODE)) {
    ignoreOpenRelease = false;
    ignoreCloseRelease = false;
    enterState(SystemState::MANUAL_IDLE);
    return;
  }

  // Check for open button release
  if (isButtonReleased(PIN_BTN_OPEN)) {
    if (!ignoreOpenRelease) {
      ignoreCloseRelease = false;
      startMovingTo(openPos);
      return;
    }
    ignoreOpenRelease = false;
  }

  // Check for close button release
  if (isButtonReleased(PIN_BTN_CLOSE)) {
    if (!ignoreCloseRelease) {
      ignoreOpenRelease = false;
      startMovingTo(closePos);
      return;
    }
    ignoreCloseRelease = false;
  }

  // Check for ToF trigger
  if (isTofTriggered()) {
    // Check if movement was interrupted and move to opposite position
    if (previousState == SystemState::TOGGLE_OPEN) {
      startMovingTo(closePos);
    } else if (previousState == SystemState::TOGGLE_CLOSE) {
      startMovingTo(openPos);
    } else {
      int64_t currentPos = encoder.getPosition();
      if (abs(currentPos - openPos) < abs(currentPos - closePos)) {
        startMovingTo(closePos);
      } else {
        startMovingTo(openPos);
      }
    }
  }
}

// Handle logic for TOGGLE_OPEN/TOGGLE_CLOSE states
static void handleToggleModeMoving() {
  int64_t currentPos = encoder.getPosition();
  // Check if already at target
  if (abs(currentPos - targetPos) <= POS_TOLERANCE) {
    motorStop();
    Serial.printf("Moved to %lld  (current: %lld)\n", targetPos, currentPos);
    enterState(SystemState::TOGGLE_IDLE);
    return;
  }

  bool stop = false;
  // Check for interruption by opposite button press
  if (currentState == SystemState::TOGGLE_OPEN && isButtonPressed(PIN_BTN_CLOSE)) {
    ignoreCloseRelease = true;
    stop = true;
  } else if (currentState == SystemState::TOGGLE_CLOSE && isButtonPressed(PIN_BTN_OPEN)) {
    ignoreOpenRelease = true;
    stop = true;
  }
  // Check for ToF trigger interruption
  else if (isTofTriggered()) {
    stop = true;
  }

  if (stop) {
    motorStop();
    enterState(SystemState::TOGGLE_IDLE);
  }
}

// Handle logic for MANUAL_IDLE/MANUAL_MOVE states
static void handleManualMode() {
  // Check for mode change
  if (isButtonReleased(PIN_BTN_MODE)) {
    motorStop();
    enterState(SystemState::TOGGLE_IDLE);
    return;
  }

  // Check for Idle state timeout
  if (currentState == SystemState::MANUAL_IDLE && (millis() - lastActivityTime) > MANUAL_TIMEOUT) {
    enterState(SystemState::TOGGLE_IDLE);
    return;
  }

  // Handle motor movement
  if (handleMotorMovement(MOTOR_DEFAULT_SPEED)) {
    if (currentState == SystemState::MANUAL_IDLE) {
      enterState(SystemState::MANUAL_MOVE);
    }
  } else {
    if (currentState == SystemState::MANUAL_MOVE) {
      enterState(SystemState::MANUAL_IDLE);
    }
  }
}

// Handle logic for CONFIG_OPEN/CONFIG_CLOSE states
static void handleConfigSetting() {
  // Check for Config mode timeout
  if ((millis() - lastActivityTime) > CONFIG_TIMEOUT) {
    motorStop();
    enterState(SystemState::TOGGLE_IDLE);
    ignoreModeRelease = false;
    return;
  }

  // Move motor
  handleMotorMovement(MOTOR_CONFIG_SPEED);

  // Check for state change
  if (isButtonReleased(PIN_BTN_MODE)) {
    // Ignore first mode button release
    if (ignoreModeRelease) {
      ignoreModeRelease = false;
      return;
    }
    motorStop();
    if (currentState == SystemState::CONFIG_OPEN) {
      tempOpenPos = encoder.getPosition();
      enterState(SystemState::CONFIG_CLOSE);
    } else {
      tempClosePos = encoder.getPosition();
      enterState(SystemState::CONFIG_SAVE);
    }
    return;
  }
}

// Handle logic for CONFIG_SAVE state
static void handleConfigModeSaving() {
  savePositions(tempOpenPos, tempClosePos);
  openPos = tempOpenPos;
  closePos = tempClosePos;
  Serial.printf("(Saved positions: Open = %lld, Close = %lld)\n", openPos, closePos);
  enterState(SystemState::TOGGLE_IDLE);
}

// Map system states to LED status
static LEDStatus setLEDState(SystemState state) {
  switch(state) {
    case SystemState::TOGGLE_IDLE:
      return STATUS_TOGGLE_IDLE;
    case SystemState::TOGGLE_OPEN:
      return STATUS_TOGGLE_OPEN;
    case SystemState::TOGGLE_CLOSE:
      return STATUS_TOGGLE_CLOSE;
    case SystemState::MANUAL_IDLE:
    case SystemState::MANUAL_MOVE:
      return STATUS_MANUAL;
    case SystemState::CONFIG_OPEN:
      return STATUS_CONFIG_OPEN;
    case SystemState::CONFIG_CLOSE:
      return STATUS_CONFIG_CLOSE;
    case SystemState::CONFIG_SAVE:
      return STATUS_CONFIG_SAVE;
    case SystemState::ERROR:
    default:
      return STATUS_ERROR;
  }
}

// Handle LED indicator logic
static void updateLedIndicator(SystemState systemState) {
  unsigned long currentTime = millis();
  LEDStatus newLedStatus = setLEDState(systemState);

  // Check if LED status has changed
  if (newLedStatus != currentLEDStatus) {
    currentLEDStatus = newLedStatus;
    statusStartTime = currentTime;
  }

  // Update LED based on current status
  switch (currentLEDStatus) {
    // LED off
    case STATUS_TOGGLE_IDLE:
      rgbLedWrite(RGB_BUILTIN, RGB_OFF);
      break;
    // Solid green
    case STATUS_TOGGLE_OPEN:
      rgbLedWrite(RGB_BUILTIN, RGB_GREEN);
      break;
    // Solid yellow
    case STATUS_TOGGLE_CLOSE:
      rgbLedWrite(RGB_BUILTIN, RGB_YELLOW);
      break;
    // Breathe orange
    case STATUS_MANUAL: {
      float pulse_norm = (sin(currentTime / 500.0f * M_PI) + 1.0f) / 2.0f;
      uint8_t brightness = 30 + (uint8_t)(pulse_norm * 180);
      rgbLedWrite(RGB_BUILTIN, brightness, (uint8_t)(brightness * 165.0 / 255.0), 0); // Breathe Orange
      break;
    }
    // Fade green/white
    case STATUS_CONFIG_OPEN: {
      uint32_t ms = currentTime % 1000;
      if (ms < 333) {
        rgbLedWrite(RGB_BUILTIN, RGB_GREEN);
      } else {
        float t;
        if (ms < 666) {
          t = (ms - 333.0f) / 333.0f;
        } else {
          t = (1000.0f - ms) / 333.0f;
        }
        uint8_t r = (uint8_t)(0 * (1.0f - t) + 255 * t);
        uint8_t b = (uint8_t)(0 * (1.0f - t) + 255 * t);
        rgbLedWrite(RGB_BUILTIN, r, 255, b);
      }
      break;
    }
    // Fade yellow/white
    case STATUS_CONFIG_CLOSE: {
      uint32_t ms = currentTime % 1000;
      if (ms < 333) {
        rgbLedWrite(RGB_BUILTIN, RGB_YELLOW);
      } else {
        float t;
        if (ms < 666) {
          t = (ms - 333.0f) / 333.0f;
        } else {
          t = (1000.0f - ms) / 333.0f;
        }
        uint8_t b = (uint8_t)(0 * (1.0f - t) + 255 * t);
        rgbLedWrite(RGB_BUILTIN, 255, 255, b);
      }
      break;
    }
    // Blink cyan twice
    case STATUS_CONFIG_SAVE: {
      uint32_t ms = currentTime % 1000;
      if (ms < 250 || (ms >= 500 && ms < 750)) {
        rgbLedWrite(RGB_BUILTIN, RGB_CYAN);
      } else {
        rgbLedWrite(RGB_BUILTIN, RGB_OFF);
      }
      break;
    }
    // Blink red
    case STATUS_ERROR:
    default: {
      uint32_t ms = currentTime % 1200;
      if (ms < 800) {
        rgbLedWrite(RGB_BUILTIN, RGB_RED);
      } else {
        rgbLedWrite(RGB_BUILTIN, RGB_OFF);
      }
      break;
    }
  }
}