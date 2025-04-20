#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

// Pin definitions
constexpr uint8_t PIN_BTN_OPEN = 19;
constexpr uint8_t PIN_BTN_MODE = 12;
constexpr uint8_t PIN_BTN_CLOSE = 18;

constexpr uint8_t PIN_MTR_IN1 = 22;
constexpr uint8_t PIN_MTR_IN2 = 21;
constexpr uint8_t PIN_MTR_PWM = 20;
constexpr uint8_t PIN_MTR_STBY = 23;
constexpr uint8_t PIN_ENC_A = 11;
constexpr uint8_t PIN_ENC_B = 10;
constexpr uint8_t ENC_PCNT = 0;

constexpr uint8_t PIN_I2C_SDA = 6;
constexpr uint8_t PIN_I2C_SCL = 7;

// System constants
constexpr uint32_t BTN_DEBOUNCE = 50;
constexpr uint32_t CONFIG_HOLD_TIME = 2000;
constexpr uint32_t MANUAL_TIMEOUT = 15000;
constexpr uint32_t CONFIG_TIMEOUT = 30000;

constexpr int MOTOR_DEFAULT_SPEED = 150;
constexpr int MOTOR_CONFIG_SPEED = 75;
constexpr int64_t POS_TOLERANCE = 40;

constexpr uint16_t TOF_THRESHOLD = 25;
constexpr uint32_t TOF_DEBOUNCE = 400;

#endif // CONFIG_H
