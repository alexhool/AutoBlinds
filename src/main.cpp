#include <Arduino.h>
#include <ESP32PCNTEncoder.h>
#include <SparkFun_TB6612.h>
#include <VL53L0X.h>

#define ENCA 11
#define ENCB 10
#define BIN1 22
#define BIN2 21
#define PWMB 20
#define STBY 23

#define WAVE_DISTANCE 25

uint8_t speed = 150;

int64_t openPos = 10000;
int64_t closePos = 0;
bool isOpen = false;

VL53L0X tof;
ESP32PCNTEncoder encoder(ENCA, ENCB);
Motor motor(BIN1, BIN2, PWMB, 1, STBY);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("\n--Setup--");

  Serial.print("Initializing ToF...");
  Wire.begin(6, 7);
  tof.setTimeout(100); // gives up after 100ms
  while (!tof.init()) {
    Serial.print(".");
    delay(500);
  }
  tof.setMeasurementTimingBudget(20000); // 20ms measurement time (default: 33ms)
  tof.startContinuous();
  Serial.println("Done");

  Serial.print("Initializing Motor...");
  motor.standby();
  while (!encoder.begin()) {
    Serial.print(".");
    delay(500);
  }
  encoder.resetPosition();
  Serial.println("Done");

  Serial.println("\n--Program--");
}

void loop() {
  if (tof.readRangeContinuousMillimeters() < WAVE_DISTANCE) {
    int64_t pos = encoder.getPosition();
    if (isOpen) {
      rgbLedWrite(RGB_BUILTIN, 255, 0, 0);
      Serial.println("Closing blinds...");
      motor.drive(-speed);
      while (pos > closePos) {
        pos = encoder.getPosition();
      }
      encoder.setPosition(closePos);
      if (pos != closePos) {
        Serial.print("Pos: ");
        Serial.println(pos);
      }
    } else {
      rgbLedWrite(RGB_BUILTIN, 0, 255, 0);
      Serial.println("Opening blinds...");
      motor.drive(speed);
      while (pos < openPos) {
        pos = encoder.getPosition();
      }
      encoder.setPosition(openPos);
      if (pos != openPos) {
        Serial.print("Pos: ");
        Serial.println(pos);
      }
    }
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    motor.brake();
    isOpen = !isOpen;
  }
  delay(10);
}