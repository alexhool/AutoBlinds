/**
 * ESP32-C6 Encoder Library using PCNT
 * 
 * Based on ESP32Encoder library by hephaestus.
 * See ESP32Encoder_LICENSE for license details.
 * This notice applies only to this file.
 */

#ifndef ESP32C6ENCODER_H
#define ESP32C6ENCODER_H

#include "driver/pulse_cnt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#define MAX_ESP32C6_ENCODERS 4 // Max number of encoders (limited by PCNT units)

// Encoder type options
enum class EncoderType {
  FULL_QUAD,    // Default
  HALF_QUAD,    
  SINGLE_EDGE
};

// Pull resistor options
enum class PullType {
  NONE,   // Default
  UP,
  DOWN
};

class ESP32C6Encoder {
public:
  // Create encoder
  ESP32C6Encoder(uint8_t pinA, uint8_t pinB, uint8_t pcntUnit = 0);

  // Delete encoder
  ~ESP32C6Encoder();

  // Set encoder type
  void setEncoderType(EncoderType type);

  // Set pull resistors
  void setPullResistors(PullType type);

  // Set glitch filter time
  void setFilter(uint32_t value_ns);

  // Start encoder
  bool begin();

  // Get current position
  int64_t getPosition();

  // Set current position
  void setPosition(int64_t position);

  // Reset position to zero
  void resetPosition();

  // Pause encoder
  esp_err_t pauseCount();

  // Resume encoder
  esp_err_t resumeCount();

private:
  EncoderType _encoderType; // Encoder type
  uint8_t _pinA;            // Encoder channel A pin
  uint8_t _pinB;            // Encoder channel B pin
  uint8_t _pcntUnit;        // PCNT unit number (0-3)
  int64_t _count;           // Extended position counter
  PullType _pullType;       // Pull resistor configuration
  uint32_t _filterTimeNs;   // Glitch filter time in nanoseconds
  bool _attached;           // Flag indicating if encoder is attached

  pcnt_unit_handle_t _pcntUnitHandle;
  pcnt_channel_handle_t _pcntChanA;
  pcnt_channel_handle_t _pcntChanB;

  static ESP32C6Encoder *encoders[MAX_ESP32C6_ENCODERS];
  static portMUX_TYPE _spinlock;

  void _applyPullResistors();
  void _configureChannels();
  bool _configureEncoder();
  static bool _pcntOverflowHandler(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx);
};

#endif // ESP32C6ENCODER_H