#include "memory.h"
#include <Arduino.h>
#include <Preferences.h>

static Preferences memory;

// Initialize nonvolatile flash memory
bool setupMemory() {
  Serial.print("Initializing Memory...");

  // Start the Preferences library
  if (!memory.begin("AutoBlinds", false)) {
    Serial.println("Failed");
    return false;
  }
  Serial.println("Done");
  return true;
}

// Load open and close positions from flash memory
void loadPositions(int64_t &openPos, int64_t &closePos) {
  // Defaults to 0 if not present
  openPos = memory.getLong64("openPosition", 0);
  closePos = memory.getLong64("closePosition", 0);
}

// Save open and close positions to flash memory
bool savePositions(int64_t openPos, int64_t closePos) {
  if (memory.putLong64("openPosition", openPos) == 0 ||
      memory.putLong64("closePosition", closePos) == 0) {
    return false;
  }
  return true;
}
