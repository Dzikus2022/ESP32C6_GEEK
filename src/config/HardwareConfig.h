#pragma once

#include <Arduino.h>

// Confirmed from official Waveshare ESP32-C6-GEEK Arduino demos
// (example/ESP32-C6-GEEK-Demo and ESP32-C6-GEEK_V2-Demo):
//   DEV_Config.h  — RST/DC/CS/BL
//   DEV_Config.cpp — SPI.begin(1, -1, 2, -1)
//   05_LCD_Button — #define PIN_INPUT 9  (active-low BOOT)
// LCD pins are the same on V1 and V2. TF/SD wiring differs — SD is unused.

namespace HardwareConfig {

constexpr int LCD_SCLK_PIN = 1;
constexpr int LCD_MOSI_PIN = 2;
constexpr int LCD_DC_PIN = 3;
constexpr int LCD_RST_PIN = 4;
constexpr int LCD_CS_PIN = 5;
constexpr int LCD_BL_PIN = 6;

constexpr int BOOT_BUTTON_PIN = 9;
constexpr bool BOOT_BUTTON_ACTIVE_LOW = true;

// Native ST7789 panel in Waveshare LCD_Driver.h
constexpr uint16_t LCD_NATIVE_WIDTH = 135;
constexpr uint16_t LCD_NATIVE_HEIGHT = 240;

}  // namespace HardwareConfig
