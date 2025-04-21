#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>

// Forward declare scheduler struct
struct ScheduleTime;

// Initialize nonvolatile memory
bool setupMemory();

// Load positions from memory
void loadPositions(int64_t &openPos, int64_t &closePos);

// Save positions to memory
bool savePositions(int64_t openPos, int64_t closePos);

// Load schedule times from memory
void loadSchedule(ScheduleTime &openSched, ScheduleTime &closeSched);

// Save schedule times to memory
bool saveSchedule(ScheduleTime openSched, ScheduleTime closeSched);

#endif // MEMORY_H
