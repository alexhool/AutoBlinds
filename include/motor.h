/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright 2025 Alexander Hool
 */

#ifndef MOTOR_H
#define MOTOR_H

// Forward declare encoder class
class ESP32PCNTEncoder;

// Declare global encoder object
extern ESP32PCNTEncoder encoder;

// Initialize motor driver pins and encoder
bool setupMotor();

// Move the motor at a given speed (-255 to 255)
void motorMove(int speed);

// Stop the motor
void motorStop();

#endif // MOTOR_H
