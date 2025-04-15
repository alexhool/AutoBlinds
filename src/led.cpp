#include "led.h"
#include "esp32-hal-rgb-led.h"

// RGB LED color definitions
#define RGB_WHITE 255, 255, 255
#define RGB_RED 255, 0, 0
#define RGB_GREEN 0, 255, 0
#define RGB_BLUE 0, 0, 255
#define RGB_YELLOW 255, 255, 0
#define RGB_CYAN 0, 255, 255
#define RGB_OFF 0, 0, 0

static LEDStatus curStatus = STATUS_IDLE;
static unsigned long statusStartTime = 0;

void setLEDStatus(LEDStatus status) {
  curStatus = status;
  statusStartTime = millis();
}

void updateLED() {
  unsigned long curTime = millis();
  
  switch (curStatus) {
    // LED off
    case STATUS_IDLE: {
      rgbLedWrite(RGB_BUILTIN, RGB_OFF);
      break;
    }

    // Solid blue
    case STATUS_SETUP: {
      rgbLedWrite(RGB_BUILTIN, RGB_BLUE);
      break;
    }

    // Solid green
    case STATUS_OPENING: {
      rgbLedWrite(RGB_BUILTIN, RGB_GREEN);
      break;
    }

    // Solid yellow
    case STATUS_CLOSING: {
      rgbLedWrite(RGB_BUILTIN, RGB_YELLOW);
      break;
    }

    // Breathe orange
    case STATUS_MANUAL: {
      uint8_t pulse = (curTime % 2000) / 8;
      if (pulse > 127) pulse = 255 - pulse;
      rgbLedWrite(RGB_BUILTIN, pulse, pulse * 80 / 255, 0);
      break;
    }

    // Fade green/white
    case STATUS_CONFIG_OPEN: {
      uint32_t ms = curTime % 1000;
      if (ms < 333) {
        rgbLedWrite(RGB_BUILTIN, RGB_GREEN);
      } else {
        float t;
        if (ms < 666) {
          t = (ms - 333) / 333.0f;
        } else {
          t = (1000 - ms) / 333.0f;
        }
        rgbLedWrite(RGB_BUILTIN, t * 255, 255, t * 255);
      }
      break;
    }

    // Fade yellow/white
    case STATUS_CONFIG_CLOSE: {
      uint32_t ms = curTime % 1000;
      if (ms < 333) {
        rgbLedWrite(RGB_BUILTIN, RGB_YELLOW);
      } else {
        float t;
        if (ms < 666) {
          t = (ms - 333) / 333.0f;
        } else {
          t = (1000 - ms) / 333.0f;
        }
        rgbLedWrite(RGB_BUILTIN, 255, 255, t * 255);
      }
      break;
    }

    // Blink teal twice
    case STATUS_SAVED: {
      uint32_t ms = curTime % 1000;
      if (ms < 250 || (ms >= 500 && ms < 750)) {
        rgbLedWrite(RGB_BUILTIN, RGB_CYAN);
      } else {
        rgbLedWrite(RGB_BUILTIN, RGB_OFF);
      }
      
      // Auto-transition to idle after 2 seconds
      if (curTime - statusStartTime >= 2000) {
        setLEDStatus(STATUS_IDLE);
      }
      break;
    }
    
    // Red for error
    case STATUS_ERROR: {
      rgbLedWrite(RGB_BUILTIN, RGB_RED);
      
      // Auto-transition to normal mode after 3 seconds
      if (curTime - statusStartTime >= 3000) {
        setLEDStatus(STATUS_IDLE);
      }
      break;
    }
  }
}