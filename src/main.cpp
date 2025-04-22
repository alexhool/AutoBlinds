/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright 2025 Alexander Hool
 */

#include <Arduino.h>
#include "memory.h"
#include "buttons.h"
#include "motor.h"
#include "tof.h"
#include "states.h"
#include "schedule.h"

void setup() {
  // Sold blue
  rgbLedWrite(RGB_BUILTIN, 0, 0, 255);
  Serial.begin(115200);
  Serial.print("\n--- Setup ---\n");

  // Initialize external components
  if (!setupMemory() || !setupMotor() || !setupTof()) {
    Serial.print("ERROR: Initialization Failed\n");
    while (true) {
      // Blink red on error
      if ((millis() % 1200) < 800) {
        rgbLedWrite(RGB_BUILTIN, 255, 0, 0);
      } else {
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
      }
      delay(2);
    }
  }
  if (!setupScheduler()) {
    Serial.print("WARNING: Network Setup Failed - Offline Mode\n");
  }

  // Initialze internal components
  setupButtons();
  setupStates();

  Serial.printf("\n--- Loop ---\n");
}

void loop() {
  unsigned long currentTime = millis();

  // Continuously update system state machine
  updateStateMachine();

  // Periodically resync RTC
  syncRTC();

  // Periodically check schedule
  checkSchedule();

  delay(2);
}
