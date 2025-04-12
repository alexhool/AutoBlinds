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
const float COUNTS_PER_REV = 2200.0; // Adjust based on your encoder
const int TEST_MOTOR_SPEED = 20;     // Moderate speed for testing
const unsigned long DIRECTION_CHANGE_INTERVAL = 5000; // 5 seconds

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
  if (currentTime - lastSpeedCalcTime >= 100) { // Update speed every 100ms
    unsigned long timeElapsed = currentTime - lastSpeedCalcTime;
    countsPerSecond = (count - lastSpeedCalcCount) * 1000.0 / timeElapsed;
    
    lastSpeedCalcTime = currentTime;
    lastSpeedCalcCount = count;
  }
  
  // Print detailed info
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
  while (!Serial) delay(10);
  
  Serial.println("ESP32C6Encoder Library Test");
  
  // Initialize motor pins
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);  // Enable motor driver
  
  // Initialize encoder with pull-ups
  //encoder.setPullResistors(PullType::UP);
  
  // Try different filter values if needed
  // encoder.setFilter(5000);  // 5 microseconds filter
  
  if (!encoder.begin()) {
    Serial.println("ERROR: Failed to initialize encoder!");
    while (1) delay(100);  // Stop if encoder init fails
  }
  
  Serial.println("Encoder initialized successfully");
  encoder.resetPosition();
  
  delay(1000); // Short delay before starting motor
  
  // Start the motor
  motorB.drive(motorSpeed);
  Serial.println("Motor started");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Print encoder information (limit rate to every 100ms)
  if (currentTime - lastPrintTime >= 100) {
    printEncoderInfo();
    lastPrintTime = currentTime;
  }
  
  // Change direction every few seconds
  if (currentTime - lastDirectionChangeTime >= DIRECTION_CHANGE_INTERVAL) {
    // Briefly stop motor before changing direction
    motorB.brake();
    delay(200);
    
    // Reverse motor direction
    motorSpeed = -motorSpeed;
    motorB.drive(motorSpeed);
    
    Serial.println("Motor direction changed");
    lastDirectionChangeTime = currentTime;
  }
  
  // Test extreme values periodically
  static bool extremeValueTestDone = false;
  if (!extremeValueTestDone && currentTime > 15000) {
    // Test setting to extreme values to verify overflow handling
    Serial.println("Testing extreme values...");
    
    // Test setting position to large positive and negative values
    encoder.setPosition(32000);
    Serial.print("Position set to 32000, actual: ");
    Serial.println(encoder.getPosition());
    delay(1000);
    
    encoder.setPosition(-32000);
    Serial.print("Position set to -32000, actual: ");
    Serial.println(encoder.getPosition());
    delay(1000);
    
    // Reset to zero
    encoder.resetPosition();
    Serial.println("Position reset to zero");
    
    extremeValueTestDone = true;
  }
  
  delay(10); // Short delay
}