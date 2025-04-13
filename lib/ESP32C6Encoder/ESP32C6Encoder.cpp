/**
 * ESP32-C6 Encoder Library using PCNT
 * 
 * Based on ESP32Encoder library by hephaestus.
 * See ESP32Encoder_LICENSE for license details.
 * This notice applies only to this file.
 */

#include "ESP32C6Encoder.h"
#include "driver/gpio.h"

// Initialize static members
ESP32C6Encoder *ESP32C6Encoder::encoders[MAX_ESP32C6_ENCODERS] = { NULL, };
portMUX_TYPE ESP32C6Encoder::_spinlock = portMUX_INITIALIZER_UNLOCKED;

// Macros for thread safety
#define _ENTER_CRITICAL() portENTER_CRITICAL_SAFE(&_spinlock)
#define _EXIT_CRITICAL() portEXIT_CRITICAL_SAFE(&_spinlock)

// Constructor
ESP32C6Encoder::ESP32C6Encoder(uint8_t pinA, uint8_t pinB, EncoderType encoderType, uint8_t pcntUnit) {
  _pinA = pinA;
  _pinB = pinB;
  _encoderType = encoderType;
  _pcntUnit = pcntUnit;
  _count = 0;                   // Initialize count to 0
  _pullType = PullType::NONE;   // Default to no pull resistors
  _filterTimeNs = 10000;        // Default filter time to 10us
  _attached = false;            // Not yet attached to PCNT
}

// Destructor
ESP32C6Encoder::~ESP32C6Encoder() {
  _ENTER_CRITICAL();
  if (_attached && _pcntUnit < MAX_ESP32C6_ENCODERS && encoders[_pcntUnit] == this) {
    encoders[_pcntUnit] = NULL;
  }
  _EXIT_CRITICAL();
  if (_attached) {
    pcnt_unit_stop(_pcntUnitHandle);
    pcnt_unit_disable(_pcntUnitHandle);
    pcnt_del_channel(_pcntChanA);
    pcnt_del_channel(_pcntChanB);
    pcnt_del_unit(_pcntUnitHandle);
  }
}

// Set up internal pull resistors for encoder pins
void ESP32C6Encoder::setPullResistors(PullType type) {
  _pullType = type;
  if (_attached) {
    _applyPullResistors();
  }
}

// Set up glitch filter to ignore short noise pulses
void ESP32C6Encoder::setFilter(uint32_t value_ns) {
  _filterTimeNs = value_ns;
  if (_attached) {
    pcnt_glitch_filter_config_t filterConfig = {
      .max_glitch_ns = value_ns,
    };
    pcnt_unit_set_glitch_filter(_pcntUnitHandle, &filterConfig);
  }
}

// Set up encoder hardware
bool ESP32C6Encoder::begin() {
  // Check if PCNT unit is valid
  if (_pcntUnit >= MAX_ESP32C6_ENCODERS) return false;

  // Check if PCNT unit is available
  _ENTER_CRITICAL();
  if (encoders[_pcntUnit] != NULL && encoders[_pcntUnit] != this) {
    _EXIT_CRITICAL();
    return false;
  }

  encoders[_pcntUnit] = this;
  _EXIT_CRITICAL();

  return _configureEncoder();
}

// Get current position
int64_t ESP32C6Encoder::getPosition() {
  int value = 0;
  int64_t result;

  _ENTER_CRITICAL();
  if (_attached) {
    pcnt_unit_get_count(_pcntUnitHandle, &value);
  }
  result = _count + (int64_t)value;
  _EXIT_CRITICAL();

  return result;
}

// Set current position
void ESP32C6Encoder::setPosition(int64_t position) {
  _ENTER_CRITICAL();
  if (_attached) {
    pcnt_unit_clear_count(_pcntUnitHandle);
  }
  _count = position;
  _EXIT_CRITICAL();
}

// Reset position to zero
void ESP32C6Encoder::resetPosition() {
  setPosition(0);
}

// Pause hardware counter
esp_err_t ESP32C6Encoder::pauseCount() {
  if (!_attached) return ESP_FAIL;
  return pcnt_unit_stop(_pcntUnitHandle);
}

// Resume hardware counter
esp_err_t ESP32C6Encoder::resumeCount() {
  if (!_attached) return ESP_FAIL;
  return pcnt_unit_start(_pcntUnitHandle);
}

// Configure PCNT channels
void ESP32C6Encoder::_configureChannels() {
  // Reset channels if they exist
  if (_pcntChanA != NULL) {
    pcnt_del_channel(_pcntChanA);
    _pcntChanA = NULL;
  }
  if (_pcntChanB != NULL) {
    pcnt_del_channel(_pcntChanB);
    _pcntChanB = NULL;
  }

  // Create channel A
  pcnt_chan_config_t chanAConfig = {
    .edge_gpio_num = _pinA,
    .level_gpio_num = _pinB,
  };
  if (pcnt_new_channel(_pcntUnitHandle, &chanAConfig, &_pcntChanA) != ESP_OK) {
    return;
  }

  // Configure based on encoder type
  switch (_encoderType) {
    case EncoderType::FULL_QUAD:
      // Create channel B
      pcnt_chan_config_t chanBConfig = {
        .edge_gpio_num = _pinB,
        .level_gpio_num = _pinA,
      };
      if (pcnt_new_channel(_pcntUnitHandle, &chanBConfig, &_pcntChanB) != ESP_OK) {
        pcnt_del_channel(_pcntChanA);
        return;
      }

      // Full quadrature mode
      pcnt_channel_set_edge_action(_pcntChanA, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
      pcnt_channel_set_level_action(_pcntChanA, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
      pcnt_channel_set_edge_action(_pcntChanB, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
      pcnt_channel_set_level_action(_pcntChanB, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP);
      break;

    case EncoderType::HALF_QUAD:
      // Half quadrature
      pcnt_channel_set_edge_action(_pcntChanA, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
      pcnt_channel_set_level_action(_pcntChanA, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
      break;

    case EncoderType::SINGLE_EDGE:
      // Single edge
      pcnt_channel_set_edge_action(_pcntChanA, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
      pcnt_channel_set_level_action(_pcntChanA, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP);
      break;
  }
}

// Interrupt for counter overflow/underflow
bool ESP32C6Encoder::_pcntOverflowHandler(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx) {
  ESP32C6Encoder *enc = static_cast<ESP32C6Encoder*>(user_ctx);
  if (enc) {
    _ENTER_CRITICAL();
    int value;
    pcnt_unit_get_count(unit, &value);  // Get current count value

    if (edata->watch_point_value == INT16_MIN) { 
      // Underflow
      enc->_count += INT16_MIN;
    } else if (edata->watch_point_value == INT16_MAX) { 
      // Overflow
      enc->_count += INT16_MAX;
    }
    _EXIT_CRITICAL();
  }
  return true;  // Keep ISR active
}

// Configure internal pull resistors for encoder pins
void ESP32C6Encoder::_applyPullResistors() {
  // Disable any existing pull resistors
  gpio_set_pull_mode((gpio_num_t)_pinA, GPIO_FLOATING);
  gpio_set_pull_mode((gpio_num_t)_pinB, GPIO_FLOATING);

  // Apply selected pull resistors
  if (_pullType == PullType::UP) {
    gpio_set_pull_mode((gpio_num_t)_pinA, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)_pinB, GPIO_PULLUP_ONLY);
  } else if (_pullType == PullType::DOWN) {
    gpio_set_pull_mode((gpio_num_t)_pinA, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode((gpio_num_t)_pinB, GPIO_PULLDOWN_ONLY);
  }
}

// Configure encoder hardware
bool ESP32C6Encoder::_configureEncoder() {

  // Configure GPIO pins
  gpio_reset_pin((gpio_num_t)_pinA);
  gpio_reset_pin((gpio_num_t)_pinB);
  gpio_set_direction((gpio_num_t)_pinA, GPIO_MODE_INPUT);
  gpio_set_direction((gpio_num_t)_pinB, GPIO_MODE_INPUT);

  // Apply pull resistors
  _applyPullResistors();

  // Configure PCNT range
  pcnt_unit_config_t unitConfig = {
    .low_limit = INT16_MIN,
    .high_limit = INT16_MAX,
  };

  // Initialize PCNT unit
  esp_err_t err = pcnt_new_unit(&unitConfig, &_pcntUnitHandle);
  if (err != ESP_OK) {
    _ENTER_CRITICAL();
    encoders[_pcntUnit] = NULL;
    _EXIT_CRITICAL();
    return false;
  }

  // Configure channels for encoder type
  _configureChannels();

  // Apply glitch filter
  setFilter(_filterTimeNs);

  // Configure overflow/underflow interrupts
  pcnt_event_callbacks_t cbs = {
    .on_reach = _pcntOverflowHandler,
  };
  pcnt_unit_register_event_callbacks(_pcntUnitHandle, &cbs, this);
  pcnt_unit_add_watch_point(_pcntUnitHandle, unitConfig.low_limit);
  pcnt_unit_add_watch_point(_pcntUnitHandle, unitConfig.high_limit);

  // Enable PCNT unit
  pcnt_unit_enable(_pcntUnitHandle);
  pcnt_unit_clear_count(_pcntUnitHandle);
  pcnt_unit_start(_pcntUnitHandle);

  _attached = true;
  return true;
}