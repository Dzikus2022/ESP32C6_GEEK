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

const char* addrTypeLabel(uint8_t type) {
  return type == 0 ? "PUBLIC" : "RANDOM";
}

void formatUptime(uint32_t seconds, char* out, size_t len) {
  const uint32_t hh = seconds / 3600UL;
  const uint32_t mm = (seconds / 60UL) % 60UL;
  const uint32_t ss = seconds % 60UL;
  snprintf(out, len, "%02lu:%02lu:%02lu", static_cast<unsigned long>(hh),
           static_cast<unsigned long>(mm), static_cast<unsigned long>(ss));
}

void formatAge(uint32_t lastSeenMs, char* out, size_t len) {
  if (lastSeenMs == 0) {
    snprintf(out, len, "--");
    return;
  }
  const uint32_t age = (millis() - lastSeenMs) / 1000UL;
  snprintf(out, len, "%lus ago", static_cast<unsigned long>(age));
}

void hexBytes(char* out, size_t len, const uint8_t* data, uint8_t n) {
  out[0] = '\0';
  for (uint8_t i = 0; i < n && (strlen(out) + 3) < len; ++i) {
    char byte[4];
    snprintf(byte, sizeof(byte), "%02X", data[i]);
    strncat(out, byte, len - strlen(out) - 1);
  }
}

void drawEmptyList(DisplayManager& display, const char* title, const char* kind) {
  display.fillBackground();
  display.drawHeader(title, "0/0");
  display.drawText(8, 36, "NO DEVICES FOUND", AppConfig::COLOR_WARN, 1);
  char line[28];
  snprintf(line, sizeof(line), "No %s yet", kind);
  display.drawText(8, 54, line, AppConfig::COLOR_MUTED, 1);
  display.drawText(8, 80, "LONG  rescan", AppConfig::COLOR_TEXT, 1);
  display.drawText(8, 96, "HOLD  back", AppConfig::COLOR_MUTED, 1);
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
  display.drawText(176, 120, "v0.1.1", AppConfig::COLOR_MUTED, 1);
}

void drawMainSystem(DisplayManager& display, const AppState& state) {
  char right[12];
  snprintf(right, sizeof(right), "v%s", AppConfig::APP_VERSION);
  display.fillBackground();
  display.drawHeader("SYSTEM", right);
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
  snprintf(line, sizeof(line), "BLE    %u", state.bleTotal);
  display.drawText(8, 72, line, AppConfig::COLOR_ACCENT, 1);
  snprintf(line, sizeof(line), "%s rev%u", state.system.chipModel,
           state.system.chipRevision);
  display.drawText(8, 96, line, AppConfig::COLOR_MUTED, 1);
  display.drawText(8, 118, "SHORT next  LONG scan", AppConfig::COLOR_MUTED, 1);
}

void drawMainWifi(DisplayManager& display, const AppState& state) {
  char right[12];
  const char* phase = phaseLabel(state.wifiPhase);
  if (phase != nullptr) {
    strncpy(right, phase, sizeof(right) - 1);
    right[sizeof(right) - 1] = '\0';
  } else {
    snprintf(right, sizeof(right), "%u", state.wifiTotal);
  }
  display.fillBackground();
  display.drawHeader("WIFI", right);
  char line[36];
  snprintf(line, sizeof(line), "Networks  %u", state.wifiTotal);
  display.drawText(8, 32, line, AppConfig::COLOR_TEXT, 1);
  if (state.wifiShown > 0) {
    snprintf(line, sizeof(line), "Best %s", state.wifi[0].ssid);
    display.drawText(8, 50, line, AppConfig::COLOR_ACCENT, 1);
    display.drawSignalBar(8, 70, state.wifi[0].rssi);
  } else {
    display.drawText(8, 50, "No APs yet", AppConfig::COLOR_MUTED, 1);
  }
  display.drawText(8, 104, "LONG  open list", AppConfig::COLOR_TEXT, 1);
  display.drawText(8, 118, "SHORT next screen", AppConfig::COLOR_MUTED, 1);
}

void drawMainBle(DisplayManager& display, const AppState& state) {
  char right[12];
  const char* phase = phaseLabel(state.blePhase);
  if (phase != nullptr) {
    strncpy(right, phase, sizeof(right) - 1);
    right[sizeof(right) - 1] = '\0';
  } else {
    snprintf(right, sizeof(right), "%u", state.bleTotal);
  }
  display.fillBackground();
  display.drawHeader("BLE", right);
  char line[36];
  snprintf(line, sizeof(line), "Devices   %u", state.bleTotal);
  display.drawText(8, 32, line, AppConfig::COLOR_TEXT, 1);
  if (state.bleShown > 0) {
    const char* label =
        state.ble[0].name[0] != '\0' ? state.ble[0].name : state.ble[0].id;
    snprintf(line, sizeof(line), "Best %s", label);
    display.drawText(8, 50, line, AppConfig::COLOR_ACCENT, 1);
    display.drawSignalBar(8, 70, state.ble[0].rssi);
  } else {
    display.drawText(8, 50, "No advertisers yet", AppConfig::COLOR_MUTED, 1);
  }
  display.drawText(8, 104, "LONG  open list", AppConfig::COLOR_TEXT, 1);
  display.drawText(8, 118, "SHORT next screen", AppConfig::COLOR_MUTED, 1);
}

void drawWifiList(DisplayManager& display, const AppState& state) {
  if (state.browse.count == 0) {
    drawEmptyList(display, "WIFI NETWORKS", "networks");
    return;
  }
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.index + 1, state.browse.count);
  display.fillBackground();
  display.drawHeader("WIFI NETWORKS", right);
  display.fillRect(0, 20, display.width(), 56, AppConfig::COLOR_HEADER);

  const WifiNetwork* row = selectedWifi(state);
  char line[40];
  if (row != nullptr) {
    snprintf(line, sizeof(line), "> %s", row->ssid);
    display.drawText(6, 28, line, AppConfig::COLOR_TEXT, 1);
    snprintf(line, sizeof(line), "  %d dBm  CH %u  %s", row->rssi, row->channel,
             wifiAuthLabel(row->authMode));
    display.drawText(6, 46, line, AppConfig::COLOR_ACCENT, 1);
    display.drawSignalBar(196, 42, row->rssi);
  } else {
    snprintf(line, sizeof(line), "> %s", state.browse.selectedKey);
    display.drawText(6, 28, line, AppConfig::COLOR_WARN, 1);
    display.drawText(6, 46, "  gone from last scan", AppConfig::COLOR_MUTED, 1);
  }
  display.drawText(8, 90, "SHORT next   LONG open", AppConfig::COLOR_MUTED, 1);
  display.drawText(8, 106, "HOLD back", AppConfig::COLOR_MUTED, 1);
}

void drawWifiDetails(DisplayManager& display, const AppState& state) {
  const WifiNetwork* row = selectedWifi(state);
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.detailPage + 1,
           AppConfig::WIFI_DETAIL_PAGES);
  display.fillBackground();
  display.drawHeader("WIFI DETAILS", right);
  if (row == nullptr) {
    display.drawText(8, 36, "Network unavailable", AppConfig::COLOR_WARN, 1);
    return;
  }
  char line[42];
  if (state.browse.detailPage == 0) {
    snprintf(line, sizeof(line), "SSID %s", row->ssid);
    display.drawText(8, 28, line, AppConfig::COLOR_TEXT, 1);
    snprintf(line, sizeof(line), "RSSI %d dBm", row->rssi);
    display.drawText(8, 46, line, AppConfig::COLOR_ACCENT, 1);
    snprintf(line, sizeof(line), "CH   %u", row->channel);
    display.drawText(8, 64, line, AppConfig::COLOR_TEXT, 1);
    snprintf(line, sizeof(line), "SEC  %s", wifiAuthLabel(row->authMode));
    display.drawText(8, 82, line, AppConfig::COLOR_TEXT, 1);
  } else {
    display.drawText(8, 28, "BSSID", AppConfig::COLOR_MUTED, 1);
    display.drawText(8, 44, row->bssid[0] != '\0' ? row->bssid : "--",
                     AppConfig::COLOR_TEXT, 1);
    display.drawText(8, 70, "No auto-connect", AppConfig::COLOR_MUTED, 1);
  }
  display.drawText(8, 118, "SHORT page  HOLD back", AppConfig::COLOR_MUTED, 1);
}

void drawBleList(DisplayManager& display, const AppState& state) {
  if (state.browse.count == 0) {
    drawEmptyList(display, "BLE DEVICES", "devices");
    return;
  }
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.index + 1, state.browse.count);
  display.fillBackground();
  display.drawHeader("BLE DEVICES", right);
  display.fillRect(0, 20, display.width(), 62, AppConfig::COLOR_HEADER);

  const BleAdvert* row = selectedBle(state);
  char line[42];
  if (row != nullptr) {
    const char* label = row->name[0] != '\0' ? row->name : row->id;
    snprintf(line, sizeof(line), "> %s", label);
    display.drawText(6, 26, line, AppConfig::COLOR_TEXT, 1);
    snprintf(line, sizeof(line), "  RSSI %d dBm", row->rssi);
    display.drawText(6, 44, line, AppConfig::COLOR_ACCENT, 1);
    display.drawText(6, 62, row->id, AppConfig::COLOR_MUTED, 1);
    display.drawSignalBar(196, 40, row->rssi);
  } else {
    snprintf(line, sizeof(line), "> %s", state.browse.selectedKey);
    display.drawText(6, 26, line, AppConfig::COLOR_WARN, 1);
    display.drawText(6, 44, "  not in last scan", AppConfig::COLOR_MUTED, 1);
  }
  display.drawText(8, 96, "SHORT next   LONG open", AppConfig::COLOR_MUTED, 1);
  display.drawText(8, 112, "HOLD back", AppConfig::COLOR_MUTED, 1);
}

void drawBleDetails(DisplayManager& display, const AppState& state) {
  const BleAdvert* row = selectedBle(state);
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.detailPage + 1,
           AppConfig::BLE_DETAIL_PAGES);
  display.fillBackground();
  display.drawHeader("BLE DETAILS", right);
  if (row == nullptr) {
    display.drawText(8, 36, state.browse.selectedKey, AppConfig::COLOR_WARN, 1);
    display.drawText(8, 54, "Device lost", AppConfig::COLOR_MUTED, 1);
    return;
  }

  char line[44];
  char extra[24];
  switch (state.browse.detailPage) {
    case 0:
      snprintf(line, sizeof(line), "%s",
               row->name[0] != '\0' ? row->name : "(no name)");
      display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
      display.drawText(8, 42, row->id, AppConfig::COLOR_ACCENT, 1);
      snprintf(line, sizeof(line), "TYPE %s", addrTypeLabel(row->addrType));
      display.drawText(8, 60, line, AppConfig::COLOR_MUTED, 1);
      snprintf(line, sizeof(line), "RSSI %d  avg %d", row->rssi, row->rssiSmooth);
      display.drawText(8, 78, line, AppConfig::COLOR_TEXT, 1);
      break;
    case 1:
      formatAge(row->lastSeenMs, extra, sizeof(extra));
      snprintf(line, sizeof(line), "SEEN %s", extra);
      display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
      snprintf(line, sizeof(line), "ADVS %u", row->advCount);
      display.drawText(8, 44, line, AppConfig::COLOR_TEXT, 1);
      snprintf(line, sizeof(line), "CONN %s",
               (row->flags & BLE_FLAG_CONNECTABLE) ? "YES" : "NO");
      display.drawText(8, 62, line, AppConfig::COLOR_TEXT, 1);
      if (row->flags & BLE_FLAG_HAS_TX) {
        snprintf(line, sizeof(line), "TX   %d dBm", row->txPower);
      } else {
        snprintf(line, sizeof(line), "TX   --");
      }
      display.drawText(8, 80, line, AppConfig::COLOR_MUTED, 1);
      break;
    case 2:
      if (row->flags & BLE_FLAG_HAS_MFG) {
        snprintf(line, sizeof(line), "MFG  0x%04X", row->manufacturerId);
        display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
        hexBytes(extra, sizeof(extra), row->mfgData, row->mfgLen);
        snprintf(line, sizeof(line), "DATA %s", extra[0] != '\0' ? extra : "--");
        display.drawText(8, 44, line, AppConfig::COLOR_MUTED, 1);
      } else {
        display.drawText(8, 26, "MFG  --", AppConfig::COLOR_MUTED, 1);
      }
      if (row->flags & BLE_FLAG_HAS_APPEAR) {
        snprintf(line, sizeof(line), "APPR 0x%04X", row->appearance);
      } else {
        snprintf(line, sizeof(line), "APPR --");
      }
      display.drawText(8, 70, line, AppConfig::COLOR_TEXT, 1);
      break;
    default:
      if (row->flags & BLE_FLAG_HAS_UUID) {
        snprintf(line, sizeof(line), "UUID %u", row->uuidCount);
        display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
        display.drawText(8, 42, row->uuid0, AppConfig::COLOR_ACCENT, 1);
        if (row->uuid1[0] != '\0') {
          display.drawText(8, 58, row->uuid1, AppConfig::COLOR_MUTED, 1);
        }
      } else {
        display.drawText(8, 26, "UUID --", AppConfig::COLOR_MUTED, 1);
      }
      if (row->flags & BLE_FLAG_HAS_SVC_DATA) {
        display.drawText(8, 80, row->svcDataUuid, AppConfig::COLOR_TEXT, 1);
        hexBytes(extra, sizeof(extra), row->svcData, row->svcDataLen);
        snprintf(line, sizeof(line), "SD   %s", extra);
        display.drawText(8, 96, line, AppConfig::COLOR_MUTED, 1);
      } else {
        display.drawText(8, 80, "SD   --", AppConfig::COLOR_MUTED, 1);
      }
      break;
  }
  display.drawText(8, 118, "SHORT page  HOLD back", AppConfig::COLOR_MUTED, 1);
}

}  // namespace

void drawCurrentScreen(DisplayManager& display, const AppState& state) {
  switch (state.screen) {
    case ScreenId::Splash:
      drawSplash(display, state);
      break;
    case ScreenId::MainWifi:
      drawMainWifi(display, state);
      break;
    case ScreenId::MainBle:
      drawMainBle(display, state);
      break;
    case ScreenId::WifiList:
      drawWifiList(display, state);
      break;
    case ScreenId::WifiDetails:
      drawWifiDetails(display, state);
      break;
    case ScreenId::BleList:
      drawBleList(display, state);
      break;
    case ScreenId::BleDetails:
      drawBleDetails(display, state);
      break;
    case ScreenId::MainSystem:
    default:
      drawMainSystem(display, state);
      break;
  }
}
