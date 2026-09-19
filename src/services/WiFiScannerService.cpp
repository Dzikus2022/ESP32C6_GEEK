#include "services/WiFiScannerService.h"

#include <WiFi.h>
#include <cstring>

#include "config/AppConfig.h"
#include "core/Log.h"

namespace {

void sortByRssi(WifiNetwork* items, uint8_t count) {
  for (uint8_t i = 1; i < count; ++i) {
    WifiNetwork key = items[i];
    int j = i - 1;
    while (j >= 0 && items[j].rssi < key.rssi) {
      items[j + 1] = items[j];
      --j;
    }
    items[j + 1] = key;
  }
}

}  // namespace

void WiFiScannerService::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  WiFi.setSleep(false);
}

bool WiFiScannerService::isBusy() const { return busy_; }

bool WiFiScannerService::startScan(AppState* state) {
  if (state == nullptr || busy_) {
    return false;
  }

  const int16_t rc = WiFi.scanNetworks(true, true);
  if (rc == WIFI_SCAN_FAILED) {
    state->wifiPhase = ScanPhase::Failed;
    logLine("WIFI", "Scan start failed");
    return false;
  }

  busy_ = true;
  startedMs_ = millis();
  state->wifiPhase = ScanPhase::Running;
  logLine("WIFI", "Scan started");
  return true;
}

void WiFiScannerService::applyResults(AppState* state) {
  const int16_t found = WiFi.scanComplete();
  if (found < 0) {
    state->wifiPhase = ScanPhase::Failed;
    state->wifiTotal = 0;
    state->wifiShown = 0;
    logLine("WIFI", "Scan failed");
    WiFi.scanDelete();
    return;
  }

  state->wifiTotal = static_cast<uint16_t>(found);
  uint8_t stored = 0;
  for (int i = 0; i < found && stored < AppConfig::WIFI_STORE_CAP; ++i) {
    WifiNetwork& row = state->wifi[stored];
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      strncpy(row.ssid, "<hidden>", sizeof(row.ssid) - 1);
    } else {
      strncpy(row.ssid, ssid.c_str(), sizeof(row.ssid) - 1);
    }
    row.ssid[sizeof(row.ssid) - 1] = '\0';
    row.rssi = static_cast<int8_t>(WiFi.RSSI(i));
    row.channel = static_cast<uint8_t>(WiFi.channel(i));
    row.authMode = static_cast<uint8_t>(WiFi.encryptionType(i));
    ++stored;
  }

  sortByRssi(state->wifi, stored);
  state->wifiShown = stored;
  state->wifiPhase = ScanPhase::Complete;
  state->wifiUpdatedMs = millis();
  logLine("WIFI", "Scan completed: %u networks", state->wifiTotal);
  WiFi.scanDelete();
}

void WiFiScannerService::tick(AppState* state) {
  if (!busy_ || state == nullptr) {
    return;
  }

  const int16_t status = WiFi.scanComplete();
  if (status >= 0 || status == WIFI_SCAN_FAILED) {
    applyResults(state);
    busy_ = false;
    state->uiDirty = true;
    return;
  }

  if ((millis() - startedMs_) > AppConfig::WIFI_SCAN_TIMEOUT_MS) {
    state->wifiPhase = ScanPhase::Failed;
    busy_ = false;
    state->uiDirty = true;
    WiFi.scanDelete();
    logLine("WIFI", "Scan timed out");
  }
}
