#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>

// Forward declare scheduler struct
struct ScheduleTime;

// Initialize nonvolatile memory
bool setupMemory();

// Load stop positions from memory
void loadPositions(int64_t &openPos, int64_t &closePos);

// Save stop positions to memory
bool savePositions(int64_t openPos, int64_t closePos);

// Load last position from memory
int64_t loadLastPosition();

// Save last position to memory
bool saveLastPosition(int64_t lastPos);

// Load schedule times from memory
void loadSchedule(ScheduleTime &openSched, ScheduleTime &closeSched);

// Save schedule times to memory
bool saveSchedule(ScheduleTime openSched, ScheduleTime closeSched);

#endif // MEMORY_H
