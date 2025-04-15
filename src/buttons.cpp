#include "buttons.h"
#include "config.h"
#include "motor.h"
#include "led.h"

// Open button variables
static bool openState = LOW;
static bool lastOpenState = LOW;
static unsigned long lastOpenTime = 0;

// Close button variables
static bool closeState = LOW;
static bool lastCloseState = LOW;
static unsigned long lastCloseTime = 0;

// Mode button variables
static bool modeState = LOW;
static bool lastModeState = LOW;
static bool modeButtonHeld = false;
static unsigned long lastModeTime = 0;
static unsigned long modeHoldTime = 0;

// Initialize button pins
void buttonsInit() {
  Serial.print("Initializing Buttons...");
  pinMode(BTN_OPEN, INPUT_PULLDOWN);
  pinMode(BTN_CLOSE, INPUT_PULLDOWN);
  pinMode(BTN_MODE, INPUT_PULLDOWN);
  Serial.println("Done");
}

// Update button states
void updateButtons() {
  unsigned long curTime = millis();

  // Open button
  bool state = digitalRead(BTN_OPEN);
  if (state != openState && curTime - lastOpenTime > BTN_DEBOUNCE) {
    openState = state;
    lastOpenTime = curTime;
  }
  
  // Close button
  state = digitalRead(BTN_CLOSE);
  if (state != closeState && curTime - lastCloseTime > BTN_DEBOUNCE) {
    closeState = state;
    lastCloseTime = curTime;
  }
  
  // Mode button
  state = digitalRead(BTN_MODE);
  if (state != modeState && curTime - lastModeTime > BTN_DEBOUNCE) {
    modeState = state;
    lastModeTime = curTime;
  }
}

// Handle button events
void handleButtons() {
  // Detect button transitions
  bool openPressed = (openState && !lastOpenState);
  bool openReleased = (!openState && lastOpenState);
  bool closePressed = (closeState && !lastCloseState);
  bool closeReleased = (!closeState && lastCloseState);
  bool modePressed = (modeState && !lastModeState);
  bool modeReleased = (!modeState && lastModeState);
  unsigned long curTime = millis();

  // Open button pressed
  if (openPressed) {
    modeHoldTime = curTime;

    if (currentMode == NORMAL) {
      if (motorState == MOTOR_CLOSING) {
          motorState = MOTOR_IDLE;
          Serial.println("Stopping blinds...");
          setLEDStatus(STATUS_IDLE);
          motorBrake();
          motorStandby();
      } else if (motorState == MOTOR_IDLE) {
        Serial.println("Opening blinds...");
        motorState = MOTOR_OPENING;
        setLEDStatus(STATUS_OPENING);
        targetPos = openPos;
        if (openPos > closePos) {
          motorDrive(MOTOR_SPEED);
        } else {
          motorDrive(-MOTOR_SPEED);
        }
      }
    } else {
      motorDrive(MOTOR_SPEED / 2);
      motorState = MOTOR_MANUAL;
    }
  }

  // Close button pressed
  if (closePressed) {
    modeHoldTime = curTime;

    if (currentMode == NORMAL) {
      if (motorState == MOTOR_OPENING) {
          Serial.println("Stopping blinds...");
          motorState = MOTOR_IDLE;
          setLEDStatus(STATUS_IDLE);
          motorBrake();
          motorStandby();
      } else if (motorState == MOTOR_IDLE) {
        Serial.println("Closing blinds...");
        motorState = MOTOR_CLOSING;
        setLEDStatus(STATUS_CLOSING);
        targetPos = closePos;
        if (closePos < openPos) {
          motorDrive(-MOTOR_SPEED);
        } else {
          motorDrive(MOTOR_SPEED);
        }
      }
    } else {
      motorDrive(-MOTOR_SPEED / 2);
      motorState = MOTOR_MANUAL;
    }
  }

  // Open/Close button released
  if (openReleased || closeReleased) {
    if (currentMode == MANUAL || currentMode == CONFIG_OPEN || currentMode == CONFIG_CLOSE) {
      if (motorState == MOTOR_MANUAL) {
        motorState = MOTOR_IDLE;
        motorBrake();
        motorStandby();
      }
    }
  }

  // Mode button
  if (modePressed) {
    modeStartTime = curTime;
    modeButtonHeld = true;
  } else if (modeReleased && modeButtonHeld) {
    // Long press
    if (curTime - modeStartTime >= HOLD_TIME && (currentMode == NORMAL || currentMode == MANUAL)) {
      // Normal/Manual -> Config_Open
      Serial.println("Config Mode");
      Serial.println("SET OPEN POSITION");
      currentMode = CONFIG_OPEN;
      modeStartTime = millis();
      setLEDStatus(STATUS_CONFIG_OPEN);
    } else {
      // Short press
      switch (currentMode) {
        // Normal -> Manual
        case NORMAL:
          Serial.println("Manual Mode");
          currentMode = MANUAL;
          modeStartTime = millis();
          setLEDStatus(STATUS_MANUAL);
          break;

        // Manual -> Normal
        case MANUAL:
          currentMode = NORMAL;
          setLEDStatus(STATUS_IDLE);
          break;

        // Config_Open -> Config_Close
        case CONFIG_OPEN:
          savePosition(encoder.getPosition(), 1);
          currentMode = CONFIG_CLOSE;
          modeStartTime = millis();
          Serial.println("SET CLOSE POSITION");
          setLEDStatus(STATUS_CONFIG_CLOSE);
          break;

        // Config_Close -> Normal
        case CONFIG_CLOSE:
          savePosition(encoder.getPosition(), 0);
          currentMode = NORMAL;
          setLEDStatus(STATUS_IDLE);
          break;
      }
    }
    modeButtonHeld = false;
  }

  // Update previous states
  lastOpenState = openState;
  lastCloseState = closeState;
  lastModeState = modeState;
}

bool getDebouncedOpen() {
  return openState;
}

bool getDebouncedClose() {
  return closeState;
}

bool getDebouncedMode() {
  return modeState;
}