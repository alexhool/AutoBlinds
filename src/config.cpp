#include "config.h"

// Pin definitions
const uint8_t PIN_BTN_OPEN = 19;
const uint8_t PIN_BTN_MODE = 12;
const uint8_t PIN_BTN_CLOSE = 18;

const uint8_t PIN_MTR_IN1 = 22;
const uint8_t PIN_MTR_IN2 = 21;
const uint8_t PIN_MTR_PWM = 20;
const uint8_t PIN_MTR_STBY = 23;
const uint8_t PIN_ENC_A = 11;
const uint8_t PIN_ENC_B = 10;
const uint8_t ENC_PCNT = 0;

const uint8_t PIN_I2C_SDA = 6;
const uint8_t PIN_I2C_SCL = 7;

// System constants
const uint32_t BTN_DEBOUNCE = 50;
const uint32_t CONFIG_HOLD_TIME = 3000;
const uint32_t MANUAL_TIMEOUT = 30000;
const uint32_t CONFIG_TIMEOUT = 60000;

const int MOTOR_DEFAULT_SPEED = 150;
const int MOTOR_CONFIG_SPEED = 75;
const int64_t POS_TOLERANCE = 15;

const uint16_t TOF_THRESHOLD = 25;
const uint32_t TOF_DEBOUNCE = 1000;
