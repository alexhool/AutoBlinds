#include "motor.h"
#include <Arduino.h>
#include <ESP32PCNTEncoder.h>
#include "config.h"

// Define global encoder object
ESP32PCNTEncoder encoder(PIN_ENC_A, PIN_ENC_B, ENC_PCNT);

// Initialize motor driver GPIO and encoder
bool setupMotor() {
  Serial.print("Initializing Motor...");

  // Set motor driver pins as outputs
  pinMode(PIN_MTR_IN1, OUTPUT);
  pinMode(PIN_MTR_IN2, OUTPUT);
  pinMode(PIN_MTR_PWM, OUTPUT);
  pinMode(PIN_MTR_STBY, OUTPUT);
  digitalWrite(PIN_MTR_STBY, HIGH);
  motorStop();

  // Set glitch filter time to ignore noise (ns)
  encoder.setFilterNs(10000);
  // Set encoder type to full quadrature (4 PPR)
  encoder.setEncoderType(EncoderType::FULL_QUAD);
  // Attempt to start encoder counting
  uint8_t attempts = 10;
  while (!encoder.begin() && attempts > 0) {
    attempts--;
    Serial.print(".");
    delay(500);
  }
  if (attempts == 0) {
    Serial.print("Failed\n");
    return false;
  }
  // Start count from 0
  encoder.resetPosition();

  Serial.print("Done\n");
  return true;
}

// Move the motor at a given speed (-255 to 255)
void motorMove(int speed) {
  // Stop motor if speed is 0
  if (speed == 0) {
    motorStop();
    return;
  }

  // Determine direction and set PWM
  if (speed > 0) {
    // Clockwise rotation
    digitalWrite(PIN_MTR_IN1, HIGH);
    digitalWrite(PIN_MTR_IN2, LOW);
    analogWrite(PIN_MTR_PWM, constrain(speed, 0, 255));
  } else {
    // Counterclockwise rotation
    digitalWrite(PIN_MTR_IN1, LOW);
    digitalWrite(PIN_MTR_IN2, HIGH);
    analogWrite(PIN_MTR_PWM, constrain(abs(speed), 0, 255));
  }
}

// Stop motor rotation
void motorStop() {
  // Set IN pins to HIGH for brake mode
  digitalWrite(PIN_MTR_IN1, HIGH);
  digitalWrite(PIN_MTR_IN2, HIGH);
  analogWrite(PIN_MTR_PWM, 0);
}
