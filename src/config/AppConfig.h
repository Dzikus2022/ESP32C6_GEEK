#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr const char* APP_NAME = "GEEK RADAR";
constexpr const char* APP_VERSION = "0.2.0";

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t SERIAL_BOOT_DELAY_MS = 2000;
constexpr uint32_t SERIAL_HEARTBEAT_MS = 10000;

constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint32_t BUTTON_LONG_PRESS_MS = 700;
constexpr uint32_t BUTTON_REPEAT_START_MS = 1100;
constexpr uint32_t BUTTON_REPEAT_INTERVAL_MS = 160;
constexpr uint32_t BUTTON_VERY_LONG_PRESS_MS = 1800;

constexpr uint32_t SPLASH_STEP_MS = 280;
constexpr uint8_t SPLASH_STEP_COUNT = 5;

constexpr uint32_t DASHBOARD_REDRAW_MS = 500;
constexpr uint32_t SCAN_STALE_MS = 20000;
constexpr uint32_t WIFI_SCAN_TIMEOUT_MS = 8000;
constexpr uint32_t BLE_SCAN_DURATION_MS = 3500;
constexpr uint32_t BLE_PROBE_TIMEOUT_MS = 6000;

constexpr uint8_t WIFI_SNIFF_CHANNEL = 6;
constexpr uint16_t WIFI_SNIFF_MAX_LEN = 256;
constexpr uint8_t WIFI_SNIFF_QUEUE = 8;

constexpr uint8_t WIFI_STORE_CAP = 24;
constexpr uint8_t BLE_STORE_CAP = 32;
constexpr uint8_t BROWSE_CAP = 32;
constexpr uint8_t RADAR_ROWS = 3;
constexpr uint8_t BLE_DETAIL_PAGES = 4;
constexpr uint8_t WIFI_DETAIL_PAGES = 2;

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
