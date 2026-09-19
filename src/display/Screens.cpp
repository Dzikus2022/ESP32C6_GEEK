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

const char* appearanceKind(uint16_t appearance, uint8_t flags) {
  if ((flags & BLE_FLAG_HAS_APPEAR) == 0) {
    return nullptr;
  }
  const uint16_t cat = static_cast<uint16_t>(appearance & 0xFFC0);
  switch (cat) {
    case 0x0040:
      return "Phone";
    case 0x0080:
      return "Computer";
    case 0x00C0:
      return "Watch";
    case 0x0140:
      return "Display";
    case 0x03C0:
      return "Audio";
    case 0x0540:
      return "Sensor";
    case 0x0940:
      return "Health";
    case 0x0C40:
      return "HID";
    case 0x0D80:
      return "Wearable";
    default:
      return nullptr;
  }
}

const char* vendorName(const BleAdvert& row) {
  if ((row.flags & BLE_FLAG_HAS_MFG) == 0) {
    return nullptr;
  }
  switch (row.manufacturerId) {
    case 0x0006:
      return "Microsoft";
    case 0x004C:
      return "Apple";
    case 0x0059:
      return "Nordic";
    case 0x0075:
      return "Samsung";
    case 0x0087:
      return "Garmin";
    case 0x00E0:
      return "Google";
    case 0x012D:
      return "Sony";
    case 0x02E5:
      return "Espressif";
    default:
      return nullptr;
  }
}

void bleFriendlyName(const BleAdvert& row, char* out, size_t len) {
  if (row.name[0] != '\0') {
    snprintf(out, len, "%s", row.name);
    return;
  }
  const char* kind = appearanceKind(row.appearance, row.flags);
  const char* vendor = vendorName(row);
  if (kind != nullptr && vendor != nullptr) {
    snprintf(out, len, "%s %s", vendor, kind);
    return;
  }
  if (kind != nullptr) {
    snprintf(out, len, "%s", kind);
    return;
  }
  if (vendor != nullptr) {
    snprintf(out, len, "%s device", vendor);
    return;
  }
  const size_t n = strlen(row.id);
  const char* tail = n > 5 ? row.id + (n - 5) : row.id;
  snprintf(out, len, "unnamed %s", tail);
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

void beginScreen(DisplayManager& display, bool full, const char* title,
                 const char* right, bool liveHeader) {
  if (full) {
    display.fillBackground();
  }
  if (full || liveHeader) {
    display.drawHeader(title, right);
  }
}

void drawEmptyList(DisplayManager& display, bool full, const char* title,
                   const char* kind) {
  beginScreen(display, full, title, "0/0", false);
  display.drawText(8, 36, "NO DEVICES FOUND", AppConfig::COLOR_WARN, 1);
  char line[28];
  snprintf(line, sizeof(line), "No %-12s yet", kind);
  display.drawText(8, 54, line, AppConfig::COLOR_MUTED, 1);
  display.drawText(8, 80, "LONG  rescan          ", AppConfig::COLOR_TEXT, 1);
  display.drawText(8, 96, "HOLD  back            ", AppConfig::COLOR_MUTED, 1);
}

void drawSplash(DisplayManager& display, const AppState& state, bool full) {
  if (full) {
    display.fillBackground();
  }
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
  char ver[12];
  snprintf(ver, sizeof(ver), "v%s", AppConfig::APP_VERSION);
  display.drawText(176, 120, ver, AppConfig::COLOR_MUTED, 1);
}

void drawMainSystem(DisplayManager& display, const AppState& state, bool full) {
  char right[12];
  snprintf(right, sizeof(right), "v%s", AppConfig::APP_VERSION);
  beginScreen(display, full, "SYSTEM", right, true);
  char line[40];
  char up[16];
  formatUptime(state.system.uptimeSec, up, sizeof(up));
  snprintf(line, sizeof(line), "UP     %-10s", up);
  display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
  snprintf(line, sizeof(line), "HEAP   %-6lu KB",
           static_cast<unsigned long>(state.system.heapFree / 1024UL));
  display.drawText(8, 40, line, AppConfig::COLOR_TEXT, 1);
  snprintf(line, sizeof(line), "WIFI   %-4u", state.wifiTotal);
  display.drawText(8, 56, line, AppConfig::COLOR_ACCENT, 1);
  snprintf(line, sizeof(line), "BLE    %-4u", state.bleTotal);
  display.drawText(8, 72, line, AppConfig::COLOR_ACCENT, 1);
  snprintf(line, sizeof(line), "ADV    %-12s",
           state.bleBeaconOn ? AppConfig::APP_NAME : "off");
  display.drawText(8, 88, line, AppConfig::COLOR_MUTED, 1);
  if (state.blePeerOn) {
    snprintf(line, sizeof(line), "PEER   %-16s", state.blePeerId);
    display.drawText(8, 104, line, AppConfig::COLOR_OK, 1);
  } else {
    display.drawText(8, 104, "PEER   --              ", AppConfig::COLOR_MUTED, 1);
  }
  if (full) {
    display.drawText(8, 118, "SHORT next  LONG scan", AppConfig::COLOR_MUTED, 1);
  }
}

void drawMainWifi(DisplayManager& display, const AppState& state, bool full) {
  char right[12];
  const char* phase = phaseLabel(state.wifiPhase);
  if (phase != nullptr) {
    strncpy(right, phase, sizeof(right) - 1);
    right[sizeof(right) - 1] = '\0';
  } else {
    snprintf(right, sizeof(right), "%u", state.wifiTotal);
  }
  beginScreen(display, full, "WIFI", right, true);
  char line[36];
  snprintf(line, sizeof(line), "Networks  %-4u", state.wifiTotal);
  display.drawText(8, 32, line, AppConfig::COLOR_TEXT, 1);
  if (state.wifiShown > 0) {
    snprintf(line, sizeof(line), "%-22s", state.wifi[0].ssid);
    display.drawText(8, 50, line, AppConfig::COLOR_ACCENT, 1);
    display.drawSignalBar(8, 70, state.wifi[0].rssi);
  } else {
    display.drawText(8, 50, "No APs yet            ", AppConfig::COLOR_MUTED, 1);
  }
  if (full) {
    display.drawText(8, 104, "LONG  open list", AppConfig::COLOR_TEXT, 1);
    display.drawText(8, 118, "SHORT next screen", AppConfig::COLOR_MUTED, 1);
  }
}

void drawMainBle(DisplayManager& display, const AppState& state, bool full) {
  char right[12];
  const char* phase = phaseLabel(state.blePhase);
  if (phase != nullptr) {
    strncpy(right, phase, sizeof(right) - 1);
    right[sizeof(right) - 1] = '\0';
  } else {
    snprintf(right, sizeof(right), "%u", state.bleTotal);
  }
  beginScreen(display, full, "BLE", right, true);
  char line[36];
  snprintf(line, sizeof(line), "Devices   %-4u", state.bleTotal);
  display.drawText(8, 32, line, AppConfig::COLOR_TEXT, 1);
  const uint8_t shown = state.bleShown < 2 ? state.bleShown : 2;
  if (shown == 0) {
    display.drawText(8, 50, "No advertisers yet    ", AppConfig::COLOR_MUTED, 1);
  } else {
    for (uint8_t i = 0; i < shown; ++i) {
      char name[28];
      bleFriendlyName(state.ble[i], name, sizeof(name));
      snprintf(line, sizeof(line), "%-16.16s %4d", name, state.ble[i].rssi);
      display.drawText(8, static_cast<int16_t>(50 + i * 16), line,
                       i == 0 ? AppConfig::COLOR_ACCENT : AppConfig::COLOR_TEXT, 1);
    }
  }
  if (state.blePeerOn) {
    display.drawText(8, 88, "PHONE CONNECTED       ", AppConfig::COLOR_OK, 1);
  } else {
    display.drawText(8, 88, "ADV GEEK RADAR        ", AppConfig::COLOR_MUTED, 1);
  }
  if (full) {
    display.drawText(8, 104, "LONG  open list", AppConfig::COLOR_TEXT, 1);
    display.drawText(8, 118, "nRF Connect -> connect", AppConfig::COLOR_MUTED, 1);
  }
}

void drawWifiList(DisplayManager& display, const AppState& state, bool full) {
  if (state.browse.count == 0) {
    drawEmptyList(display, full, "WIFI NETWORKS", "networks");
    return;
  }
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.index + 1, state.browse.count);
  beginScreen(display, full, "WIFI NETWORKS", right, false);

  char line[40];
  for (uint8_t i = 0; i < AppConfig::RADAR_ROWS; ++i) {
    const uint8_t pos = static_cast<uint8_t>(state.browse.index + i);
    const int16_t y = static_cast<int16_t>(24 + i * 18);
    if (pos >= state.browse.count) {
      display.drawText(6, y, "                      ", AppConfig::COLOR_MUTED, 1);
      continue;
    }
    const int idx = findWifiIndex(state, state.browse.keys[pos]);
    const char mark = (i == 0) ? '>' : ' ';
    if (idx < 0) {
      snprintf(line, sizeof(line), "%c %-16.16s     ", mark, "gone");
      display.drawText(6, y, line, AppConfig::COLOR_WARN, 1);
      continue;
    }
    const WifiNetwork& row = state.wifi[static_cast<uint8_t>(idx)];
    snprintf(line, sizeof(line), "%c %-16.16s %4d", mark, row.ssid, row.rssi);
    display.drawText(6, y, line,
                     i == 0 ? AppConfig::COLOR_ACCENT : AppConfig::COLOR_TEXT, 1);
  }
  if (full) {
    display.drawText(8, 90, "SHORT next   LONG open", AppConfig::COLOR_MUTED, 1);
    display.drawText(8, 106, "HOLD back", AppConfig::COLOR_MUTED, 1);
  }
}

void drawWifiDetails(DisplayManager& display, const AppState& state, bool full) {
  const WifiNetwork* row = selectedWifi(state);
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.detailPage + 1,
           AppConfig::WIFI_DETAIL_PAGES);
  beginScreen(display, full, "WIFI DETAILS", right, false);
  if (row == nullptr) {
    display.drawText(8, 36, "Network unavailable  ", AppConfig::COLOR_WARN, 1);
    return;
  }
  char line[42];
  if (state.browse.detailPage == 0) {
    snprintf(line, sizeof(line), "SSID %-16s", row->ssid);
    display.drawText(8, 28, line, AppConfig::COLOR_TEXT, 1);
    snprintf(line, sizeof(line), "RSSI %d dBm   ", row->rssi);
    display.drawText(8, 46, line, AppConfig::COLOR_ACCENT, 1);
    snprintf(line, sizeof(line), "CH   %-4u", row->channel);
    display.drawText(8, 64, line, AppConfig::COLOR_TEXT, 1);
    snprintf(line, sizeof(line), "SEC  %-6s", wifiAuthLabel(row->authMode));
    display.drawText(8, 82, line, AppConfig::COLOR_TEXT, 1);
  } else if (full) {
    display.drawText(8, 28, "BSSID", AppConfig::COLOR_MUTED, 1);
    display.drawText(8, 44, row->bssid[0] != '\0' ? row->bssid : "--",
                     AppConfig::COLOR_TEXT, 1);
    display.drawText(8, 70, "No auto-connect", AppConfig::COLOR_MUTED, 1);
  }
  if (full) {
    display.drawText(8, 118, "SHORT page  HOLD back", AppConfig::COLOR_MUTED, 1);
  }
}

void drawBleList(DisplayManager& display, const AppState& state, bool full) {
  if (state.browse.count == 0) {
    drawEmptyList(display, full, "BLE DEVICES", "devices");
    return;
  }
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.index + 1, state.browse.count);
  beginScreen(display, full, "BLE DEVICES", right, false);

  char line[42];
  char name[28];
  for (uint8_t i = 0; i < AppConfig::RADAR_ROWS; ++i) {
    const uint8_t pos = static_cast<uint8_t>(state.browse.index + i);
    const int16_t y = static_cast<int16_t>(24 + i * 18);
    if (pos >= state.browse.count) {
      display.drawText(6, y, "                      ", AppConfig::COLOR_MUTED, 1);
      continue;
    }
    const int idx = findBleIndex(state, state.browse.keys[pos]);
    const char mark = (i == 0) ? '>' : ' ';
    if (idx < 0) {
      snprintf(line, sizeof(line), "%c %-16.16s     ", mark, "gone");
      display.drawText(6, y, line, AppConfig::COLOR_WARN, 1);
      continue;
    }
    const BleAdvert& row = state.ble[static_cast<uint8_t>(idx)];
    bleFriendlyName(row, name, sizeof(name));
    snprintf(line, sizeof(line), "%c %-16.16s %4d", mark, name, row.rssi);
    display.drawText(6, y, line,
                     i == 0 ? AppConfig::COLOR_ACCENT : AppConfig::COLOR_TEXT, 1);
  }
  if (full) {
    display.drawText(8, 90, "SHORT next   LONG open", AppConfig::COLOR_MUTED, 1);
    display.drawText(8, 106, "HOLD back", AppConfig::COLOR_MUTED, 1);
  }
}

void drawBleDetails(DisplayManager& display, const AppState& state, bool full) {
  const BleAdvert* row = selectedBle(state);
  char right[12];
  snprintf(right, sizeof(right), "%u/%u", state.browse.detailPage + 1,
           AppConfig::BLE_DETAIL_PAGES);
  beginScreen(display, full, "BLE DETAILS", right, false);
  if (row == nullptr) {
    display.drawText(8, 36, state.browse.selectedKey, AppConfig::COLOR_WARN, 1);
    display.drawText(8, 54, "Device lost           ", AppConfig::COLOR_MUTED, 1);
    return;
  }

  char line[44];
  char extra[24];
  char name[32];
  bleFriendlyName(*row, name, sizeof(name));
  switch (state.browse.detailPage) {
    case 0:
      snprintf(line, sizeof(line), "%-22s", name);
      display.drawText(8, 26, line, AppConfig::COLOR_ACCENT, 1);
      snprintf(line, sizeof(line), "%-22s",
               row->name[0] != '\0' ? row->id : "no advertised name");
      display.drawText(8, 42, line, AppConfig::COLOR_MUTED, 1);
      snprintf(line, sizeof(line), "TYPE %-8s", addrTypeLabel(row->addrType));
      display.drawText(8, 60, line, AppConfig::COLOR_MUTED, 1);
      snprintf(line, sizeof(line), "RSSI %d  avg %d  ", row->rssi, row->rssiSmooth);
      display.drawText(8, 78, line, AppConfig::COLOR_TEXT, 1);
      break;
    case 1:
      formatAge(row->lastSeenMs, extra, sizeof(extra));
      snprintf(line, sizeof(line), "SEEN %-12s", extra);
      display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
      snprintf(line, sizeof(line), "ADVS %-6u", row->advCount);
      display.drawText(8, 44, line, AppConfig::COLOR_TEXT, 1);
      snprintf(line, sizeof(line), "CONN %s ",
               (row->flags & BLE_FLAG_CONNECTABLE) ? "YES" : "NO");
      display.drawText(8, 62, line, AppConfig::COLOR_TEXT, 1);
      if (row->flags & BLE_FLAG_HAS_TX) {
        snprintf(line, sizeof(line), "TX   %d dBm  ", row->txPower);
      } else {
        snprintf(line, sizeof(line), "TX   --      ");
      }
      display.drawText(8, 80, line, AppConfig::COLOR_MUTED, 1);
      break;
    case 2:
      if (row->flags & BLE_FLAG_HAS_MFG) {
        const char* vendor = vendorName(*row);
        snprintf(line, sizeof(line), "MFG  0x%04X %-10s", row->manufacturerId,
                 vendor != nullptr ? vendor : "");
        display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
        hexBytes(extra, sizeof(extra), row->mfgData, row->mfgLen);
        snprintf(line, sizeof(line), "DATA %-16s", extra[0] != '\0' ? extra : "--");
        display.drawText(8, 44, line, AppConfig::COLOR_MUTED, 1);
      } else if (full) {
        display.drawText(8, 26, "MFG  --               ", AppConfig::COLOR_MUTED, 1);
      }
      if (row->flags & BLE_FLAG_HAS_APPEAR) {
        const char* kind = appearanceKind(row->appearance, row->flags);
        snprintf(line, sizeof(line), "APPR 0x%04X %-10s", row->appearance,
                 kind != nullptr ? kind : "");
      } else {
        snprintf(line, sizeof(line), "APPR --              ");
      }
      display.drawText(8, 70, line, AppConfig::COLOR_TEXT, 1);
      break;
    default:
      if (row->flags & BLE_FLAG_HAS_UUID) {
        snprintf(line, sizeof(line), "UUID %u  ", row->uuidCount);
        display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
        display.drawText(8, 42, row->uuid0, AppConfig::COLOR_ACCENT, 1);
        if (row->uuid1[0] != '\0') {
          display.drawText(8, 58, row->uuid1, AppConfig::COLOR_MUTED, 1);
        }
      } else if (full) {
        display.drawText(8, 26, "UUID --", AppConfig::COLOR_MUTED, 1);
      }
      if (row->flags & BLE_FLAG_HAS_SVC_DATA) {
        display.drawText(8, 80, row->svcDataUuid, AppConfig::COLOR_TEXT, 1);
        hexBytes(extra, sizeof(extra), row->svcData, row->svcDataLen);
        snprintf(line, sizeof(line), "SD   %-16s", extra);
        display.drawText(8, 96, line, AppConfig::COLOR_MUTED, 1);
      } else if (full) {
        display.drawText(8, 80, "SD   --", AppConfig::COLOR_MUTED, 1);
      }
      break;
  }
  const char* probe = "LONG probe  HOLD back";
  uint16_t probeColor = AppConfig::COLOR_MUTED;
  switch (state.bleProbe) {
    case BleProbePhase::Connecting:
      probe = "PROBE ...            ";
      probeColor = AppConfig::COLOR_ACCENT;
      break;
    case BleProbePhase::Ok:
      probe = "PROBE OK   HOLD back";
      probeColor = AppConfig::COLOR_OK;
      break;
    case BleProbePhase::Refused:
      probe = "PROBE REFUSED  HOLD ";
      probeColor = AppConfig::COLOR_WARN;
      break;
    case BleProbePhase::Timeout:
      probe = "PROBE TIMEOUT  HOLD ";
      probeColor = AppConfig::COLOR_WARN;
      break;
    case BleProbePhase::Idle:
    default:
      break;
  }
  display.drawText(8, 118, probe, probeColor, 1);
}

void drawWifiSniffer(DisplayManager& display, const AppState& state, bool full) {
  const WifiSniffStats& s = state.wifiSniff;
  beginScreen(display, full, "WIFI SNIFFER", s.streaming ? "LIVE" : "IDLE",
              true);
  char line[40];
  snprintf(line, sizeof(line), "CH   %-4u  PKT/s %-5u", s.channel, s.pps);
  display.drawText(8, 26, line, AppConfig::COLOR_TEXT, 1);
  snprintf(line, sizeof(line), "MGMT %-6lu CTRL %-6lu",
           static_cast<unsigned long>(s.mgmt),
           static_cast<unsigned long>(s.ctrl));
  display.drawText(8, 42, line, AppConfig::COLOR_ACCENT, 1);
  snprintf(line, sizeof(line), "DATA %-6lu RSSI %d  ",
           static_cast<unsigned long>(s.data), s.rssi);
  display.drawText(8, 58, line, AppConfig::COLOR_TEXT, 1);
  snprintf(line, sizeof(line), "RX   %-6lu DROP %-6lu",
           static_cast<unsigned long>(s.received),
           static_cast<unsigned long>(s.dropped));
  display.drawText(8, 74, line, AppConfig::COLOR_MUTED, 1);
  snprintf(line, sizeof(line), "USB  %-6lu %s",
           static_cast<unsigned long>(s.streamed),
           s.streaming ? "LIVE" : "IDLE");
  display.drawText(8, 90, line,
                   s.streaming ? AppConfig::COLOR_OK : AppConfig::COLOR_MUTED, 1);
  if (full) {
    display.drawText(8, 118, "LONG stream  HOLD back", AppConfig::COLOR_MUTED,
                     1);
  }
}

}  // namespace

void drawCurrentScreen(DisplayManager& display, const AppState& state,
                       bool fullClear) {
  switch (state.screen) {
    case ScreenId::Splash:
      drawSplash(display, state, fullClear);
      break;
    case ScreenId::MainWifi:
      drawMainWifi(display, state, fullClear);
      break;
    case ScreenId::MainBle:
      drawMainBle(display, state, fullClear);
      break;
    case ScreenId::WifiList:
      drawWifiList(display, state, fullClear);
      break;
    case ScreenId::WifiDetails:
      drawWifiDetails(display, state, fullClear);
      break;
    case ScreenId::BleList:
      drawBleList(display, state, fullClear);
      break;
    case ScreenId::BleDetails:
      drawBleDetails(display, state, fullClear);
      break;
    case ScreenId::WifiSniffer:
      drawWifiSniffer(display, state, fullClear);
      break;
    case ScreenId::MainSystem:
    default:
      drawMainSystem(display, state, fullClear);
      break;
  }
}
