#include "tof.h"
#include "config.h"
#include <VL53L0X.h>
#include <Wire.h>

static VL53L0X tof;
static unsigned long lastWaveTime = 0;

// Initialize ToF sensor
void tofInit() {
  Serial.print("Initializing ToF...");

  Wire.begin(I2C_SDA, I2C_SCL);
  tof.setTimeout(100); // gives up after 100ms
  while (!tof.init()) {
    Serial.print(".");
    delay(250);
  }
  tof.setMeasurementTimingBudget(20000); // 20ms measurement time
  tof.startContinuous();

  Serial.println("Done");
}

// Detect wave
bool tofWave() {
  if (currentMode != NORMAL) {
    return false;
  }

  unsigned long curTime = millis();

  // Debounce wave detection
  if (curTime - lastWaveTime < WAVE_DEBOUNCE) {
    return false;
  }

  // Check if distance is in range
  if (tof.readRangeContinuousMillimeters() < WAVE_DISTANCE) {
    lastWaveTime = curTime;
    return true;
  }

  return false;
}