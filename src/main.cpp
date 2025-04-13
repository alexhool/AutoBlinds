#include <Arduino.h>
#include <SparkFun_TB6612.h>
#include <ESP32C6Encoder.h>

// === Pin Config ===
const int ENCODER_PIN_A = 11;
const int ENCODER_PIN_B = 10;

const int BIN1 = 22;
const int BIN2 = 21;
const int PWMB = 20;
const int STBY = 23;
const int offsetB = 1;

// === Objects ===
ESP32C6Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
Motor motorB(BIN1, BIN2, PWMB, offsetB, STBY);

// === Settings ===
const int MOTOR_SPEED = 200;
const int TEST_DURATION_MS = 5000;

void runMotorTest(const char* label) {
  Serial.printf("\n=== %s ===\n", label);
  encoder.resetPosition();

  motorB.drive(MOTOR_SPEED);
  unsigned long start = millis();

  while (millis() - start < TEST_DURATION_MS) {
    Serial.printf("Count: %lld\n", encoder.getPosition());
    delay(200);
  }

  motorB.brake();
  delay(300);

  motorB.drive(-MOTOR_SPEED);
  start = millis();

  while (millis() - start < TEST_DURATION_MS) {
    Serial.printf("Count: %lld\n", encoder.getPosition());
    delay(200);
  }

  motorB.brake();
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);  // Enable motor driver

  encoder.setPullResistors(PullType::UP);
  encoder.setFilter(10000);

  if (!encoder.begin()) {
    Serial.println("Failed to initialize encoder.");
    while (true) delay(100);
  }

  encoder.setEncoderType(EncoderType::FULL_QUAD);
  runMotorTest("FULL QUAD");

  encoder.setEncoderType(EncoderType::HALF_QUAD);
  runMotorTest("HALF QUAD");

  encoder.setEncoderType(EncoderType::SINGLE_EDGE);
  runMotorTest("SINGLE EDGE");

  Serial.println("\n=== All Tests Complete ===");
}

void loop() {
  // Run once
}
