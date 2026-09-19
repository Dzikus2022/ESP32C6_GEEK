#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr const char* APP_NAME = "GEEK RADAR";
constexpr const char* APP_VERSION = "0.1.0";

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t SERIAL_BOOT_DELAY_MS = 2000;
constexpr uint32_t SERIAL_HEARTBEAT_MS = 10000;

constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint32_t BUTTON_LONG_PRESS_MS = 700;

constexpr uint32_t SPLASH_STEP_MS = 280;
constexpr uint8_t SPLASH_STEP_COUNT = 5;

constexpr uint32_t DASHBOARD_REDRAW_MS = 500;
constexpr uint32_t SCAN_STALE_MS = 20000;
constexpr uint32_t WIFI_SCAN_TIMEOUT_MS = 8000;
constexpr uint32_t BLE_SCAN_DURATION_MS = 3500;

constexpr uint8_t WIFI_STORE_CAP = 12;
constexpr uint8_t BLE_STORE_CAP = 12;
constexpr uint8_t RADAR_ROWS = 5;

constexpr uint16_t COLOR_BG = 0x0843;
constexpr uint16_t COLOR_HEADER = 0x02B5;
constexpr uint16_t COLOR_TEXT = 0xFFFF;
constexpr uint16_t COLOR_MUTED = 0x8C71;
constexpr uint16_t COLOR_ACCENT = 0x07FF;
constexpr uint16_t COLOR_OK = 0x07E0;
constexpr uint16_t COLOR_WARN = 0xFE60;
constexpr uint16_t COLOR_BAR = 0x07E0;
constexpr uint16_t COLOR_BAR_EMPTY = 0x2945;

}  // namespace AppConfig
