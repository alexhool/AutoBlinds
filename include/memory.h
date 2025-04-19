#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>

// Initialize nonvolatile memory
void setupMemory();

// Load positions from memory
void loadPositions(int64_t &openPos, int64_t &closePos);

// Save positions to memory
void savePositions(int64_t openPos, int64_t closePos);

#endif // MEMORY_H
