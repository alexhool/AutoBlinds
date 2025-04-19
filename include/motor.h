#ifndef MOTOR_H
#define MOTOR_H

// Forward declare encoder class
class ESP32PCNTEncoder;

// Declare global encoder object
extern ESP32PCNTEncoder encoder;

// Initialize motor driver pins and encoder
void setupMotor();

// Move the motor at a given speed (-255 to 255)
void motorMove(int speed);

// Stop the motor
void motorStop();

#endif // MOTOR_H
