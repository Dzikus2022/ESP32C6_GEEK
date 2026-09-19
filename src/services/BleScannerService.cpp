#include "services/BleScannerService.h"

#include <NimBLEDevice.h>
#include <cstring>

#include "config/AppConfig.h"
#include "core/Log.h"

namespace {

struct BleScratch {
  BleAdvert items[AppConfig::BLE_STORE_CAP];
  uint8_t count = 0;
  uint16_t seen = 0;
  bool done = false;
  bool failed = false;
};

BleScratch gScratch;

void sortByRssi(BleAdvert* items, uint8_t count) {
  for (uint8_t i = 1; i < count; ++i) {
    BleAdvert key = items[i];
    int j = i - 1;
    while (j >= 0 && items[j].rssi < key.rssi) {
      items[j + 1] = items[j];
      --j;
    }
    items[j + 1] = key;
  }
}

int findById(const char* id) {
  for (uint8_t i = 0; i < gScratch.count; ++i) {
    if (strncmp(gScratch.items[i].id, id, sizeof(gScratch.items[i].id)) == 0) {
      return i;
    }
  }
  return -1;
}

void remember(const char* name, const char* id, int8_t rssi) {
  ++gScratch.seen;
  const int existing = findById(id);
  if (existing >= 0) {
    if (rssi > gScratch.items[existing].rssi) {
      gScratch.items[existing].rssi = rssi;
      if (name[0] != '\0') {
        strncpy(gScratch.items[existing].name, name,
                sizeof(gScratch.items[existing].name) - 1);
      }
    }
    return;
  }

  if (gScratch.count >= AppConfig::BLE_STORE_CAP) {
    int weakest = 0;
    for (uint8_t i = 1; i < gScratch.count; ++i) {
      if (gScratch.items[i].rssi < gScratch.items[weakest].rssi) {
        weakest = i;
      }
    }
    if (rssi <= gScratch.items[weakest].rssi) {
      return;
    }
    strncpy(gScratch.items[weakest].id, id,
            sizeof(gScratch.items[weakest].id) - 1);
    strncpy(gScratch.items[weakest].name, name,
            sizeof(gScratch.items[weakest].name) - 1);
    gScratch.items[weakest].rssi = rssi;
    return;
  }

  BleAdvert& row = gScratch.items[gScratch.count++];
  strncpy(row.id, id, sizeof(row.id) - 1);
  row.id[sizeof(row.id) - 1] = '\0';
  strncpy(row.name, name, sizeof(row.name) - 1);
  row.name[sizeof(row.name) - 1] = '\0';
  row.rssi = rssi;
}

class RadarScanCallbacks : public NimBLEScanCallbacks {
 public:
  void onResult(const NimBLEAdvertisedDevice* device) override {
    if (device == nullptr) {
      return;
    }
    char id[18] = {};
    strncpy(id, device->getAddress().toString().c_str(), sizeof(id) - 1);
    char name[24] = {};
    if (device->haveName()) {
      strncpy(name, device->getName().c_str(), sizeof(name) - 1);
    }
    remember(name, id, static_cast<int8_t>(device->getRSSI()));
  }

  void onScanEnd(const NimBLEScanResults& /*results*/, int reason) override {
    gScratch.failed = (reason != 0 && gScratch.seen == 0);
    gScratch.done = true;
  }
};

RadarScanCallbacks gCallbacks;

}  // namespace

bool BleScannerService::begin() {
  NimBLEDevice::init("");
  NimBLEScan* scan = NimBLEDevice::getScan();
  if (scan == nullptr) {
    logLine("BLE", "Scan object missing");
    ready_ = false;
    return false;
  }
  scan->setScanCallbacks(&gCallbacks, false);
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);
  scan->setMaxResults(0);
  ready_ = true;
  logLine("BLE", "Ready");
  return true;
}

bool BleScannerService::isBusy() const { return busy_; }

bool BleScannerService::startScan(AppState* state) {
  if (!ready_ || busy_ || state == nullptr) {
    return false;
  }

  NimBLEScan* scan = NimBLEDevice::getScan();
  if (scan == nullptr) {
    state->blePhase = ScanPhase::Failed;
    logLine("BLE", "Scan start failed");
    return false;
  }

  gScratch = BleScratch{};
  if (!scan->start(AppConfig::BLE_SCAN_DURATION_MS, false)) {
    state->blePhase = ScanPhase::Failed;
    logLine("BLE", "Scan start failed");
    return false;
  }

  busy_ = true;
  startedMs_ = millis();
  state->blePhase = ScanPhase::Running;
  logLine("BLE", "Scan started");
  return true;
}

void BleScannerService::tick(AppState* state) {
  if (!busy_ || state == nullptr) {
    return;
  }

  const bool timedOut =
      (millis() - startedMs_) > (AppConfig::BLE_SCAN_DURATION_MS + 1500);
  if (!gScratch.done && !timedOut) {
    return;
  }

  if (timedOut && !gScratch.done) {
    NimBLEScan* scan = NimBLEDevice::getScan();
    if (scan != nullptr) {
      scan->stop();
    }
    gScratch.done = true;
  }

  sortByRssi(gScratch.items, gScratch.count);
  state->bleTotal = gScratch.seen;
  state->bleShown = gScratch.count;
  for (uint8_t i = 0; i < gScratch.count; ++i) {
    state->ble[i] = gScratch.items[i];
  }
  state->blePhase = gScratch.failed ? ScanPhase::Failed : ScanPhase::Complete;
  state->bleUpdatedMs = millis();
  state->uiDirty = true;
  busy_ = false;
  logLine("BLE", "Scan completed: %u devices", state->bleTotal);
}
