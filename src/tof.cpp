/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright 2025 Alexander Hool
 */

#include "tof.h"
#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>
#include "config.h"

static VL53L0X tof;
static bool wasTriggered = false;
static unsigned long lastTriggerTime = 0;

// Initialize ToF sensor via I2C
bool setupTof() {
  Serial.print("Initializing ToF...");

  // Initialize I2C communication
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  // Set sensor timeout (ms)
  tof.setTimeout(500);
  // Attempt to start ToF sensor
  uint8_t attempts = 10;
  while (!tof.init() && attempts > 0) {
    attempts--;
    Serial.print(".");
    delay(500);
  }
  if (attempts == 0) {
    Serial.print("Failed\n");
    return false;
  }
  // Set single measurement time (us)
  tof.setMeasurementTimingBudget(40000);
  // Start continuous measurements
  tof.startContinuous();

  Serial.print("Done\n");
  return true;
}

// Detect if an object just appeared within the threshold distance
bool isTofTriggered() {
  unsigned long currentTime = millis();
  uint16_t distance = tof.readRangeContinuousMillimeters();
  bool isTriggered = false;
  bool trigger = false;

  // Object detected if within threshold
  isTriggered = (!tof.timeoutOccurred() && distance < TOF_THRESHOLD && distance > 0);

  // Debounce object detection
  if ((currentTime - lastTriggerTime) > TOF_DEBOUNCE) {
    // Check if object just appeared
    if (isTriggered && !wasTriggered) {
      trigger = true;
      // Store current time for next call
      lastTriggerTime = currentTime;
      Serial.printf("ToF Triggered: %u mm\n", distance);
    }
  }

  // Store current state for next call
  wasTriggered = isTriggered;

  return trigger;
}
