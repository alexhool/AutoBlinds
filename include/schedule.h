#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <cstdint>

// Scheduler time information
struct ScheduleTime {
  uint8_t hour;
  uint8_t minute;
};

// Initialize scheduler, Wi-Fi, NTP, and web server
bool setupScheduler();

// Check scheduled times for action triggers
void checkSchedule();

// Sync RTC with NTP server
void syncRTC();

#endif // SCHEDULE_H
