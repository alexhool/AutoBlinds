#include "memory.h"
#include <Arduino.h>
#include <Preferences.h>
#include "schedule.h"

static Preferences memory;

// Initialize nonvolatile flash memory
bool setupMemory() {
  Serial.print("Initializing Memory...");

  // Start Preferences library
  if (!memory.begin("AutoBlinds", false)) {
    Serial.print("Failed\n");
    return false;
  }
  Serial.print("Done\n");
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

// Load last encoder position from flash memory
int64_t loadLastPosition() {
  // Defaults to 0 if not present
  return memory.getLong64("lastPosition", 0);
}

// Save last encoder position to flash memory
bool saveLastPosition(int64_t lastPos) {
  if (memory.putLong64("lastPosition", lastPos) == 0) {
    return false;
  }
  return true;
}

// Load open and close schedule times from flash memory
void loadSchedule(ScheduleTime &openSched, ScheduleTime &closeSched) {
  // Defaults to 99 if not present
  openSched.hour = memory.getInt("openScheduleHour", 99);
  openSched.minute = memory.getInt("openScheduleMinute", 99);
  closeSched.hour = memory.getInt("closeScheduleHour", 99);
  closeSched.minute = memory.getInt("closeScheduleMinute", 99);
}

// Save open and close schedule times to flash memory
bool saveSchedule(ScheduleTime openSched, ScheduleTime closeSched) {
  if (memory.putInt("openScheduleHour", openSched.hour) == 0 ||
      memory.putInt("openScheduleMinute", openSched.minute) == 0 ||
      memory.putInt("closeScheduleHour", closeSched.hour) == 0 ||
      memory.putInt("closeScheduleMinute", closeSched.minute) == 0) {
    return false;
  }
  return true;
}
