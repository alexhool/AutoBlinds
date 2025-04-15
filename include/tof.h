#ifndef TOF_H
#define TOF_H

#include <Arduino.h>

// I2C pins
#define I2C_SDA 6
#define I2C_SCL 7

// ToF constants
#define WAVE_DISTANCE 25
#define WAVE_DEBOUNCE 1000

// Initialize ToF sensor
void tofInit();

// Detect wave
bool tofWave();

#endif // TOF_H