#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

// Pin definitions
extern const uint8_t PIN_BTN_OPEN;
extern const uint8_t PIN_BTN_CLOSE;
extern const uint8_t PIN_BTN_MODE;

extern const uint8_t PIN_MTR_IN1;
extern const uint8_t PIN_MTR_IN2;
extern const uint8_t PIN_MTR_PWM;
extern const uint8_t PIN_MTR_STBY;

extern const uint8_t PIN_ENC_A;
extern const uint8_t PIN_ENC_B;
extern const uint8_t ENC_PCNT;

extern const uint8_t PIN_I2C_SDA;
extern const uint8_t PIN_I2C_SCL;

// System constants
extern const uint32_t BTN_DEBOUNCE;
extern const uint32_t CONFIG_HOLD_TIME;
extern const uint32_t MANUAL_TIMEOUT;
extern const uint32_t CONFIG_TIMEOUT;

extern const int MOTOR_DEFAULT_SPEED;
extern const int MOTOR_CONFIG_SPEED;
extern const int64_t POS_TOLERANCE;

extern const uint16_t TOF_THRESHOLD;
extern const uint32_t TOF_DEBOUNCE;

#endif // CONFIG_H
