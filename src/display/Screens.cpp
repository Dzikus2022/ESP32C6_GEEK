#include "display/Screens.h"

#include <WiFi.h>
#include <cstdio>
#include <cstring>

#include "config/AppConfig.h"

namespace {

const char* wifiAuthLabel(uint8_t auth) {
  return auth == WIFI_AUTH_OPEN ? "OPEN" : "SEC";
}

const char* phaseLabel(ScanPhase phase) {
  switch (phase) {
    case ScanPhase::Running:
      return "SCAN";
    case ScanPhase::Failed:
      return "FAIL";
    case ScanPhase::Complete:
      return nullptr;
    case ScanPhase::Idle:
    default:
      return "IDLE";
  }
}

void formatUptime(uint32_t seconds, char* out, size_t len) {
  const uint32_t hh = seconds / 3600UL;
  const uint32_t mm = (seconds / 60UL) % 60UL;
  const uint32_t ss = seconds % 60UL;
  snprintf(out, len, "%02lu:%02lu:%02lu", static_cast<unsigned long>(hh),
           static_cast<unsigned long>(mm), static_cast<unsigned long>(ss));
}

void drawSplash(DisplayManager& display, const AppState& state) {
  display.fillBackground();
  display.drawText(18, 18, AppConfig::APP_NAME, AppConfig::COLOR_ACCENT, 2);
  display.drawText(18, 42, "ESP32-C6", AppConfig::COLOR_TEXT, 1);
  display.drawText(18, 54, "Wi-Fi 6 | BLE 5", AppConfig::COLOR_MUTED, 1);

  if (state.splashStep >= 1) {
    display.drawText(18, 74, "SYSTEM ........ OK", AppConfig::COLOR_OK, 1);
  }
  if (state.splashStep >= 2) {
    display.drawText(18, 88,
                     state.displayOk ? "DISPLAY ....... OK" : "DISPLAY ..... FAIL",
                     state.displayOk ? AppConfig::COLOR_OK : AppConfig::COLOR_WARN,
                     1);
  }
  if (state.splashStep >= 3) {
    display.drawText(18, 102,
                     state.radioReady ? "RADIO ......... OK" : "RADIO ....... WAIT",
                     state.radioReady ? AppConfig::COLOR_OK : AppConfig::COLOR_WARN,
                     1);
  }
  display.drawText(180, 120, "v0.1.0", AppConfig::COLOR_MUTED, 1);
}

void drawDashboard(DisplayManager& display, const AppState& state) {
  char right[12];
  snprintf(right, sizeof(right), "v%s", AppConfig::APP_VERSION);
  display.fillBackground();
  display.drawHeader(AppConfig::APP_NAME, right);

  char line[40];
  char up[16];
  formatUptime(state.system.uptimeSec, up, sizeof(up));
  snprintf(line, sizeof(line), "UP     %s", up);
  display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);

  snprintf(line, sizeof(line), "HEAP   %lu KB",
           static_cast<unsigned long>(state.system.heapFree / 1024UL));
  display.drawText(8, 40, line, AppConfig::COLOR_TEXT, 1);

  snprintf(line, sizeof(line), "WIFI   %u", state.wifiTotal);
  display.drawText(8, 56, line, AppConfig::COLOR_ACCENT, 1);
  display.drawSignalBar(90, 52, state.wifiShown > 0 ? state.wifi[0].rssi : -90);

  snprintf(line, sizeof(line), "BLE    %u", state.bleTotal);
  display.drawText(8, 76, line, AppConfig::COLOR_ACCENT, 1);
  display.drawSignalBar(90, 72, state.bleShown > 0 ? state.ble[0].rssi : -90);

  snprintf(line, sizeof(line), "%s rev%u  %luMB", state.system.chipModel,
           state.system.chipRevision,
           static_cast<unsigned long>(state.system.flashBytes / (1024UL * 1024UL)));
  display.drawText(8, 104, line, AppConfig::COLOR_MUTED, 1);
  display.drawText(8, 118, "BOOT short:page  long:scan", AppConfig::COLOR_MUTED, 1);
}

void drawWifi(DisplayManager& display, const AppState& state) {
  char right[12];
  const char* phase = phaseLabel(state.wifiPhase);
  if (phase != nullptr) {
    strncpy(right, phase, sizeof(right) - 1);
    right[sizeof(right) - 1] = '\0';
  } else {
    snprintf(right, sizeof(right), "%u", state.wifiTotal);
  }

  display.fillBackground();
  display.drawHeader("WIFI RADAR", right);

  const uint8_t rows =
      state.wifiShown < AppConfig::RADAR_ROWS ? state.wifiShown : AppConfig::RADAR_ROWS;
  if (rows == 0) {
    display.drawText(8, 40, "No networks yet", AppConfig::COLOR_MUTED, 1);
    display.drawText(8, 56, "Long-press to scan", AppConfig::COLOR_MUTED, 1);
    return;
  }

  for (uint8_t i = 0; i < rows; ++i) {
    const WifiNetwork& row = state.wifi[i];
    const int16_t y = static_cast<int16_t>(24 + i * 20);
    char ssid[11];
    strncpy(ssid, row.ssid, 10);
    ssid[10] = '\0';
    char left[28];
    snprintf(left, sizeof(left), "%-10s %4d %s", ssid, row.rssi,
             wifiAuthLabel(row.authMode));
    display.drawText(6, y, left, AppConfig::COLOR_TEXT, 1);
    display.drawSignalBar(196, static_cast<int16_t>(y - 2), row.rssi);
  }
}

void drawBle(DisplayManager& display, const AppState& state) {
  char right[12];
  const char* phase = phaseLabel(state.blePhase);
  if (phase != nullptr) {
    strncpy(right, phase, sizeof(right) - 1);
    right[sizeof(right) - 1] = '\0';
  } else {
    snprintf(right, sizeof(right), "%u", state.bleTotal);
  }

  display.fillBackground();
  display.drawHeader("BLE RADAR", right);

  const uint8_t rows =
      state.bleShown < AppConfig::RADAR_ROWS ? state.bleShown : AppConfig::RADAR_ROWS;
  if (rows == 0) {
    display.drawText(8, 40, "No advertisers yet", AppConfig::COLOR_MUTED, 1);
    display.drawText(8, 56, "Long-press to scan", AppConfig::COLOR_MUTED, 1);
    return;
  }

  for (uint8_t i = 0; i < rows; ++i) {
    const BleAdvert& row = state.ble[i];
    const int16_t y = static_cast<int16_t>(24 + i * 20);
    const char* label = row.name[0] != '\0' ? row.name : row.id;
    char name[12];
    strncpy(name, label, 11);
    name[11] = '\0';
    char left[24];
    snprintf(left, sizeof(left), "%-11s %4d", name, row.rssi);
    display.drawText(6, y, left, AppConfig::COLOR_TEXT, 1);
    display.drawSignalBar(196, static_cast<int16_t>(y - 2), row.rssi);
  }
}

}  // namespace

void drawCurrentScreen(DisplayManager& display, const AppState& state) {
  switch (state.screen) {
    case ScreenId::Splash:
      drawSplash(display, state);
      break;
    case ScreenId::WifiRadar:
      drawWifi(display, state);
      break;
    case ScreenId::BleRadar:
      drawBle(display, state);
      break;
    case ScreenId::Dashboard:
    default:
      drawDashboard(display, state);
      break;
  }
}
