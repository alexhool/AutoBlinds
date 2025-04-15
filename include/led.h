#ifndef LED_H
#define LED_H

// LED status codes
enum LEDStatus {
  STATUS_IDLE,
  STATUS_SETUP,
  STATUS_OPENING,
  STATUS_CLOSING,
  STATUS_MANUAL,
  STATUS_CONFIG_OPEN,
  STATUS_CONFIG_CLOSE,
  STATUS_SAVED,
  STATUS_ERROR
};

// Set the LED Status
void setLEDStatus(LEDStatus status);

// Update LED based on current status
void updateLED();

#endif // LED_H