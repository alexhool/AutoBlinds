# AutoBlinds
### ECE 4180 - Embedded Systems Project

---

## Project Overview



---

## System Components

* **Microcontroller (ESP32-C6-DevKitC-1):**
* **DC Motor w/ Encoder (JGY-370-EN):**
* **Motor Driver (TB6612FNG):**
* **Time-of-Flight Sensor (VL53L0X):**
* **Timekeeping:**
* **Web Interface:**
* **Tactile Switches:**
* **RGB LED:**
* **Power Regulation (Buck Converter):**

---

## Problems Encountered

* 

---

## Comparison to Real-World Systems

* 

---

## Potential Improvements

* 

---

## Dependencies

* **Preferences:**
    * Part of the Arduino Core for ESP32, used for non-volatile storage
    * License: [LGPL-2.1](https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md) ([github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32))
* **ESP32PCNTEncoder (Custom Library):**
    * Derived from the ESP32Encoder library by hephaestus
    * License: [LGPL-3.0](LICENSE) (`LICENSE`)
    * Base Library
        * Copyright (C) 2018 hephaestus
        * License: [BSD-like](lib/ESP32PCNTEncoder/LICENSE_ESP32Encoder) (`lib/ESP32PCNTEncoder/LICENSE_ESP32Encoder`)
* **VL53L0X:**
    * Copyright:
        * (C) 2017-2022 Pololu Corporation
        * (C) 2016 STMicroelectronics International N.V.
    * License: [MIT+BSD-like](.pio/libdeps/esp32-c6-devkitc-1/VL53L0X/LICENSE.txt) (`.pio/.../VL53L0X/LICENSE.txt`)
* **WiFiManager:**
    * Copyright (C) 2015 tzapu
    * License: [MIT](.pio/libdeps/esp32-c6-devkitc-1/WiFiManager/LICENSE) (`.pio/.../WiFiManager/LICENSE`)
* **AsyncTCP:**
    * Copyright (C) 2016-2025 Hristo Gochkov, Mathieu Carbou, Emil Muratov
    * License: [LGPL-3.0](.pio/libdeps/esp32-c6-devkitc-1/AsyncTCP/LICENSE) (`.pio/.../AsyncTCP/LICENSE`)
* **ESPAsyncWebServer:**
    * Copyright (C) 2016-2025 Hristo Gochkov, Mathieu Carbou, Emil Muratov
    * License: [LGPL-3.0](.pio/libdeps/esp32-c6-devkitc-1/ESPAsyncWebServer/LICENSE) (`.pio/.../ESPAsyncWebServer/LICENSE`)
