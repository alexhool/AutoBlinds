#include "motor.h"
#include <Arduino.h>
#include <ESP32PCNTEncoder.h>
#include "config.h"

// Define global encoder object
ESP32PCNTEncoder encoder(PIN_ENC_A, PIN_ENC_B, ENC_PCNT);

// Initialize motor driver GPIO and encoder
void setupMotor() {
  Serial.print("Initializing Motor...");

  // Set motor driver pins as outputs
  pinMode(PIN_MTR_IN1, OUTPUT);
  pinMode(PIN_MTR_IN2, OUTPUT);
  pinMode(PIN_MTR_PWM, OUTPUT);
  pinMode(PIN_MTR_STBY, OUTPUT);
  motorStop();

  // Set glitch filter time to ignore noise (ns)
  encoder.setFilterNs(10000);
  // Set encoder type to full quadrature (4 PPR)
  encoder.setEncoderType(EncoderType::FULL_QUAD);
  while (!encoder.begin()) {
    Serial.print(".");
    delay(100);
  }
  // Start count from 0
  encoder.resetPosition();

  Serial.println("Done");
}

// Move the motor at a given speed (-255 to 255)
void motorMove(int speed) {
  // Stop and enter standby if speed is 0
  if (speed == 0) {
    motorStop();
    return;
  }

  // Take driver out of standby
  digitalWrite(PIN_MTR_STBY, HIGH);
  delayMicroseconds(10);

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

// Stop motor rotation and put driver in low-power standby
void motorStop() {
  // Set IN pins the same for high impedance
  digitalWrite(PIN_MTR_IN1, LOW);
  digitalWrite(PIN_MTR_IN2, LOW);
  analogWrite(PIN_MTR_PWM, 0);
  // Set driver to standby mode
  digitalWrite(PIN_MTR_STBY, LOW);
}
