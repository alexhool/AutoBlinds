#ifndef STATES_H
#define STATES_H

#include <cstdint>

// System operational states
enum class SystemState {
  TOGGLE_IDLE,    // 0 - Default
  TOGGLE_OPEN,    // 1
  TOGGLE_CLOSE,   // 2
  MANUAL_IDLE,    // 3
  MANUAL_MOVE,    // 4
  CONFIG_OPEN,    // 5
  CONFIG_CLOSE,   // 6
  CONFIG_SAVE,    // 7
  ERROR           // 8
};

// Initialize state machine
void setupStates();

// Handle system state transitions and logic
void updateStateMachine();

// Transition to a new state and update LED
void enterState(SystemState newState);

// Start moving to open position
void triggerOpen();

// Start moving to close position
void triggerClose();

#endif // STATES_H
