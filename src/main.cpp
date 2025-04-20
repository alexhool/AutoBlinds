#include <Arduino.h>
#include "memory.h"
#include "buttons.h"
#include "motor.h"
#include "tof.h"
#include "states.h"

void setup() {
  // Sold blue
  rgbLedWrite(RGB_BUILTIN, 0, 0, 255);
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- Setup ---");

  // Initialize external components
  if (!setupMemory() || !setupMotor() || !setupTof()) {
    Serial.println("Initialization Failed");
    while (true) {
      // Blink red
      if ((millis() % 1200) < 800) {
        rgbLedWrite(RGB_BUILTIN, 255, 0, 0);
      } else {
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
      }
    }
  }

  // Initialze internal components
  setupButtons();
  setupStates();

  Serial.printf("\n--- Loop ---\n");
}

void loop() {
  // Continuously update system state machine
  updateStateMachine();
  delay(1);
}
