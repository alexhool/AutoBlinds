#include <Arduino.h>
#include "memory.h"
#include "buttons.h"
#include "motor.h"
#include "tof.h"
#include "states.h"

void setup() {
  rgbLedWrite(RGB_BUILTIN, 0, 0, 255);
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n--- Setup ---");

  // Initialize components
  setupMemory();
  setupButtons();
  setupMotor();
  setupTof();
  setupStates();

  Serial.printf("\n--- Program ---\n");
}

void loop() {
  // Update system state machine
  updateStateMachine();
  delay(1);
}
