#include "memory.h"
#include <Arduino.h>
#include <Preferences.h>

static Preferences memory;

// Initialize nonvolatile flash memory
void setupMemory() {
  Serial.print("Initializing Memory...");

  // Start the Preferences library
  memory.begin("AutoBlinds", false);

  Serial.println("Done");
}

// Load open and close positions from flash memory
void loadPositions(int64_t &openPos, int64_t &closePos) {
  // Defaults to 0 if not present
  openPos = memory.getLong64("openPosition", 0);
  closePos = memory.getLong64("closePosition", 0);
  Serial.printf("Loaded positions: Open=%lld, Close=%lld\n", openPos, closePos);
}

// Save open and close positions to flash memory
void savePositions(int64_t openPos, int64_t closePos) {
  memory.putLong64("openPosition", openPos);
  memory.putLong64("closePosition", closePos);
  Serial.printf("Saved positions: Open=%lld, Close=%lld\n", openPos, closePos);
}
