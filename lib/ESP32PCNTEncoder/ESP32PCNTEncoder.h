/**
 * ESP32 Encoder Library using PCNT
 * 
 * Based on ESP32Encoder library by hephaestus.
 * See LICENSE_ESP32Encoder for license details.
 * This notice applies to this file.
 */

#ifndef ESP32PCNTENCODER_H
#define ESP32PCNTENCODER_H

#include "driver/pulse_cnt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#if CONFIG_IDF_TARGET_ESP32
    #define MAX_ESP32_ENCODERS 8
#elif CONFIG_IDF_TARGET_ESP32S2
    #define MAX_ESP32_ENCODERS 4
#elif CONFIG_IDF_TARGET_ESP32S3
    #define MAX_ESP32_ENCODERS 4
#elif CONFIG_IDF_TARGET_ESP32C3
    #define MAX_ESP32_ENCODERS 0
#elif CONFIG_IDF_TARGET_ESP32C5
    #define MAX_ESP32_ENCODERS 4
#elif CONFIG_IDF_TARGET_ESP32C6
    #define MAX_ESP32_ENCODERS 4
#elif CONFIG_IDF_TARGET_ESP32H2
    #define MAX_ESP32_ENCODERS 4
#elif CONFIG_IDF_TARGET_ESP32P4
    #define MAX_ESP32_ENCODERS 4
#else
    #define MAX_ESP32_ENCODERS 4   // Default
#endif

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

class ESP32PCNTEncoder {
public:
  // Create encoder
  ESP32PCNTEncoder(uint8_t pinA, uint8_t pinB, uint8_t pcntUnit = 0);

  // Delete encoder
  ~ESP32PCNTEncoder();

  // Set encoder type
  void setEncoderType(EncoderType type);

  // Set pull resistors
  void setPullResistors(PullType type);

  // Set glitch filter time
  void setFilterNs(uint32_t value_ns);

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

  static ESP32PCNTEncoder *encoders[MAX_ESP32_ENCODERS];
  static portMUX_TYPE _spinlock;

  void _applyPullResistors();
  void _configureChannels();
  bool _configureEncoder();
  static bool _pcntOverflowHandler(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx);
};

#endif // ESP32PCNTENCODER_H