#include "tof.h"
#include <Arduino.h>
#include <VL53L0X.h>
#include "config.h"

static VL53L0X tof;
static bool wasTriggered = false;
static unsigned long lastTriggerTime = 0;

// Initialize ToF sensor via I2C
void setupTof() {
  Serial.print("Initializing ToF...");

  // Initialize I2C communication
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  // Set sensor timeout (ms)
  tof.setTimeout(500);
  while (!tof.init()) {
    Serial.print(".");
    delay(100);
  }
  // Set single measurement time (us)
  tof.setMeasurementTimingBudget(50000);
  // Start continuous measurements
  tof.startContinuous();

  Serial.println("Done");
}

// Detect if an object just appeared within the threshold distance
bool isTofTriggered() {
  uint16_t distance = tof.readRangeContinuousMillimeters();
  unsigned long currentTime = millis();
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
