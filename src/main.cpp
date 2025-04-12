#include <Arduino.h>
#include <SparkFun_TB6612.h>
#include "ESP32C6Encoder.h"

// Define pins
#define ENCODER_PIN_A 11
#define ENCODER_PIN_B 10

// Motor pins
const int BIN1 = 22;
const int BIN2 = 21;
const int PWMB = 20;
const int STBY = 23;
const int offsetB = 1;

// Create encoder and motor instances
ESP32C6Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
Motor motorB = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// Constants
const float COUNTS_PER_REV = 2200.0;  // Adjust based on your encoder
const int TEST_MOTOR_SPEED = 30;      // Motor speed for testing
const unsigned long DIRECTION_CHANGE_INTERVAL = 5000;  // 5 seconds

// Variables
unsigned long lastDirectionChangeTime = 0;
int motorSpeed = TEST_MOTOR_SPEED;
int64_t lastCount = 0;
unsigned long lastPrintTime = 0;

void printEncoderInfo() {
  int64_t count = encoder.getPosition();
  float rotations = count / COUNTS_PER_REV;
  
  // Calculate speed in counts per second
  static unsigned long lastSpeedCalcTime = 0;
  static int64_t lastSpeedCalcCount = 0;
  static float countsPerSecond = 0;
  
  unsigned long currentTime = millis();
  if (currentTime - lastSpeedCalcTime >= 100) {  // Update speed every 100ms
    unsigned long timeElapsed = currentTime - lastSpeedCalcTime;
    countsPerSecond = (count - lastSpeedCalcCount) * 1000.0 / timeElapsed;
    
    lastSpeedCalcTime = currentTime;
    lastSpeedCalcCount = count;
  }
  
  // Print status info
  Serial.print("Count: ");
  Serial.print(count);
  Serial.print(" | Delta: ");
  Serial.print(count - lastCount);
  Serial.print(" | Rotations: ");
  Serial.print(rotations, 4);
  Serial.print(" | Speed: ");
  Serial.print(countsPerSecond);
  Serial.println(" counts/sec");
  
  lastCount = count;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n=== ESP32-C6 Encoder Test ===");
  
  // Initialize motor pins
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);  // Enable motor driver
  
  // Configure the encoder
  encoder.setPullResistors(PullType::UP);  // Use pull-ups (important!)
  encoder.setFilter(10000);  // 10 microseconds filter
  
  // If direction is backward, uncomment this:
  // encoder.setReverseDirection(true);
  
  if (!encoder.begin()) {
    Serial.println("ERROR: Failed to initialize encoder!");
    while (1) {
      delay(100);
    }
  }
  
  Serial.println("Encoder initialized successfully");
  encoder.resetPosition();
  
  delay(1000);
  
  // Start the motor
  motorB.drive(motorSpeed);
  Serial.println("Motor started at speed: " + String(motorSpeed));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Print encoder information every 200ms
  if (currentTime - lastPrintTime >= 200) {
    printEncoderInfo();
    lastPrintTime = currentTime;
  }
  
  // Change motor direction every 5 seconds
  if (currentTime - lastDirectionChangeTime >= DIRECTION_CHANGE_INTERVAL) {
    // Briefly stop motor before changing direction
    motorB.brake();
    delay(200);
    
    // Reverse motor direction
    motorSpeed = -motorSpeed;
    motorB.drive(motorSpeed);
    
    Serial.print("Motor direction changed to: ");
    Serial.println(motorSpeed);
    
    lastDirectionChangeTime = currentTime;
  }
  
  delay(10);
}