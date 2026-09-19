#include "services/BleScannerService.h"

#include <NimBLEDevice.h>
#include <cstdio>
#include <cstring>
#include <string>

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

void copyText(char* dest, size_t destLen, const char* src) {
  strncpy(dest, src != nullptr ? src : "", destLen - 1);
  dest[destLen - 1] = '\0';
}

void copyPrintableName(char* dest, size_t destLen, const char* src) {
  if (destLen == 0) {
    return;
  }
  size_t out = 0;
  for (size_t i = 0; src != nullptr && src[i] != '\0' && out + 1 < destLen; ++i) {
    const unsigned char c = static_cast<unsigned char>(src[i]);
    dest[out++] = (c >= 32 && c < 127) ? static_cast<char>(c) : '?';
  }
  dest[out] = '\0';
}

void copyUuid(char* dest, size_t destLen, const NimBLEUUID& uuid) {
  const std::string text = uuid.toString();
  copyText(dest, destLen, text.c_str());
}

void fillFromDevice(BleAdvert* out, const NimBLEAdvertisedDevice* device) {
  *out = BleAdvert{};
  copyText(out->id, sizeof(out->id), device->getAddress().toString().c_str());
  const std::string advertisedName = device->getName();
  if (!advertisedName.empty()) {
    copyPrintableName(out->name, sizeof(out->name), advertisedName.c_str());
  }
  out->rssi = device->getRSSI();
  out->rssiSmooth = out->rssi;
  out->addrType = device->getAddressType();
  if (device->isConnectable()) {
    out->flags |= BLE_FLAG_CONNECTABLE;
  }
  if (device->haveTXPower()) {
    out->flags |= BLE_FLAG_HAS_TX;
    out->txPower = device->getTXPower();
  }
  if (device->haveAppearance()) {
    out->flags |= BLE_FLAG_HAS_APPEAR;
    out->appearance = device->getAppearance();
  }
  if (device->haveManufacturerData()) {
    const std::string mfg = device->getManufacturerData();
    if (mfg.size() >= 2) {
      out->flags |= BLE_FLAG_HAS_MFG;
      out->manufacturerId = static_cast<uint8_t>(mfg[0]) |
                            (static_cast<uint16_t>(static_cast<uint8_t>(mfg[1])) << 8);
      out->mfgLen = static_cast<uint8_t>(
          mfg.size() - 2 > sizeof(out->mfgData) ? sizeof(out->mfgData)
                                                : mfg.size() - 2);
      memcpy(out->mfgData, mfg.data() + 2, out->mfgLen);
    }
  }
  out->uuidCount = device->getServiceUUIDCount();
  if (out->uuidCount > 0) {
    out->flags |= BLE_FLAG_HAS_UUID;
    copyUuid(out->uuid0, sizeof(out->uuid0), device->getServiceUUID(0));
    if (out->uuidCount > 1) {
      copyUuid(out->uuid1, sizeof(out->uuid1), device->getServiceUUID(1));
    }
  }
  if (device->haveServiceData() && device->getServiceDataCount() > 0) {
    out->flags |= BLE_FLAG_HAS_SVC_DATA;
    copyUuid(out->svcDataUuid, sizeof(out->svcDataUuid),
             device->getServiceDataUUID(0));
    const std::string data = device->getServiceData(static_cast<uint8_t>(0));
    out->svcDataLen = static_cast<uint8_t>(
        data.size() > sizeof(out->svcData) ? sizeof(out->svcData) : data.size());
    memcpy(out->svcData, data.data(), out->svcDataLen);
  }
  out->lastSeenMs = millis();
  out->advCount = 1;
}

void mergeAdvert(BleAdvert* dst, const BleAdvert& src) {
  if (src.name[0] != '\0') {
    copyPrintableName(dst->name, sizeof(dst->name), src.name);
  }
  dst->rssi = src.rssi;
  dst->rssiSmooth = static_cast<int8_t>((dst->rssiSmooth * 3 + src.rssi) / 4);
  dst->addrType = src.addrType;
  dst->flags |= src.flags;
  if (src.flags & BLE_FLAG_HAS_TX) {
    dst->txPower = src.txPower;
  }
  if (src.flags & BLE_FLAG_HAS_APPEAR) {
    dst->appearance = src.appearance;
  }
  if (src.flags & BLE_FLAG_HAS_MFG) {
    dst->manufacturerId = src.manufacturerId;
    dst->mfgLen = src.mfgLen;
    memcpy(dst->mfgData, src.mfgData, src.mfgLen);
  }
  if (src.flags & BLE_FLAG_HAS_UUID) {
    dst->uuidCount = src.uuidCount;
    copyText(dst->uuid0, sizeof(dst->uuid0), src.uuid0);
    copyText(dst->uuid1, sizeof(dst->uuid1), src.uuid1);
  }
  if (src.flags & BLE_FLAG_HAS_SVC_DATA) {
    copyText(dst->svcDataUuid, sizeof(dst->svcDataUuid), src.svcDataUuid);
    dst->svcDataLen = src.svcDataLen;
    memcpy(dst->svcData, src.svcData, src.svcDataLen);
  }
  dst->lastSeenMs = millis();
  if (dst->advCount < 65535) {
    ++dst->advCount;
  }
}

int findScratch(const char* id) {
  for (uint8_t i = 0; i < gScratch.count; ++i) {
    if (strncmp(gScratch.items[i].id, id, sizeof(gScratch.items[i].id)) == 0) {
      return i;
    }
  }
  return -1;
}

void remember(const BleAdvert& incoming) {
  ++gScratch.seen;
  const int existing = findScratch(incoming.id);
  if (existing >= 0) {
    mergeAdvert(&gScratch.items[existing], incoming);
    return;
  }
  if (gScratch.count >= AppConfig::BLE_STORE_CAP) {
    int weakest = 0;
    for (uint8_t i = 1; i < gScratch.count; ++i) {
      if (gScratch.items[i].rssi < gScratch.items[weakest].rssi) {
        weakest = i;
      }
    }
    if (incoming.rssi <= gScratch.items[weakest].rssi) {
      return;
    }
    gScratch.items[weakest] = incoming;
    return;
  }
  gScratch.items[gScratch.count++] = incoming;
}

class RadarScanCallbacks : public NimBLEScanCallbacks {
 public:
  void onResult(const NimBLEAdvertisedDevice* device) override {
    if (device == nullptr) {
      return;
    }
    BleAdvert row;
    fillFromDevice(&row, device);
    remember(row);
  }

  void onScanEnd(const NimBLEScanResults& /*results*/, int reason) override {
    gScratch.failed = (reason != 0 && gScratch.seen == 0);
    gScratch.done = true;
  }
};

RadarScanCallbacks gCallbacks;

enum class ProbeEvent : uint8_t { None, Ok, Fail };

volatile ProbeEvent gProbeEvent = ProbeEvent::None;
NimBLEClient* gProbeClient = nullptr;

class ProbeCallbacks : public NimBLEClientCallbacks {
 public:
  void onConnect(NimBLEClient* client) override {
    gProbeEvent = ProbeEvent::Ok;
    if (client != nullptr) {
      client->disconnect();
    }
  }

  void onConnectFail(NimBLEClient* /*client*/, int /*reason*/) override {
    gProbeEvent = ProbeEvent::Fail;
  }

  void onPassKeyEntry(NimBLEConnInfo& /*info*/) override {
    gProbeEvent = ProbeEvent::Fail;
    if (gProbeClient != nullptr) {
      gProbeClient->disconnect();
    }
  }
};

ProbeCallbacks gProbeCallbacks;

volatile bool gPeerOn = false;
char gPeerId[18] = {};

class RadarServerCallbacks : public NimBLEServerCallbacks {
 public:
  void onConnect(NimBLEServer* /*server*/, NimBLEConnInfo& info) override {
    gPeerOn = true;
    copyText(gPeerId, sizeof(gPeerId), info.getAddress().toString().c_str());
    logLine("BLE", "Peer in %s", gPeerId);
  }

  void onDisconnect(NimBLEServer* /*server*/, NimBLEConnInfo& /*info*/,
                    int /*reason*/) override {
    gPeerOn = false;
    gPeerId[0] = '\0';
    logLine("BLE", "Peer out");
  }
};

RadarServerCallbacks gServerCallbacks;

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

}  // namespace

void BleScannerService::startBeacon() {
  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  if (adv == nullptr) {
    return;
  }
  adv->setName(AppConfig::APP_NAME);
  adv->enableScanResponse(true);
  if (!adv->isAdvertising()) {
    adv->start();
  }
}

void BleScannerService::stopBeacon() {
  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  if (adv != nullptr && adv->isAdvertising()) {
    adv->stop();
  }
}

void BleScannerService::hush() {
  stopScanIfRunning();
  stopBeacon();
}

void BleScannerService::unhush() {
  if (ready_ && !probing_) {
    startBeacon();
  }
}

bool BleScannerService::begin() {
  NimBLEDevice::init(AppConfig::APP_NAME);
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

  NimBLEServer* server = NimBLEDevice::createServer();
  if (server != nullptr) {
    server->setCallbacks(&gServerCallbacks, false);
    server->createService("180A");
    server->start();
  }
  startBeacon();

  ready_ = true;
  logLine("BLE", "Ready, advertising %s", AppConfig::APP_NAME);
  return true;
}

bool BleScannerService::isBusy() const { return busy_ || probing_; }

void BleScannerService::stopScanIfRunning() {
  if (!busy_) {
    return;
  }
  NimBLEScan* scan = NimBLEDevice::getScan();
  if (scan != nullptr) {
    scan->stop();
  }
  gScratch.done = true;
  busy_ = false;
}

void BleScannerService::finishProbe(AppState* state, BleProbePhase result) {
  if (state != nullptr) {
    state->bleProbe = result;
    state->bleProbeAtMs = millis();
    state->uiDirty = true;
  }
  probing_ = false;
  gProbeEvent = ProbeEvent::None;
  startBeacon();
  logLine("BLE", "Probe %u", static_cast<unsigned>(result));
}

bool BleScannerService::startProbe(AppState* state, const BleAdvert& target) {
  if (!ready_ || probing_ || state == nullptr || target.id[0] == '\0') {
    return false;
  }

  stopScanIfRunning();
  stopBeacon();

  if (gProbeClient == nullptr) {
    gProbeClient = NimBLEDevice::createClient();
  }
  if (gProbeClient == nullptr) {
    finishProbe(state, BleProbePhase::Refused);
    logLine("BLE", "Probe client missing");
    return false;
  }

  if (gProbeClient->isConnected()) {
    gProbeClient->disconnect();
  }

  gProbeEvent = ProbeEvent::None;
  gProbeClient->setClientCallbacks(&gProbeCallbacks, false);
  gProbeClient->setConnectTimeout(AppConfig::BLE_PROBE_TIMEOUT_MS);
  gProbeClient->setConnectRetries(0);

  const NimBLEAddress addr(std::string(target.id), target.addrType);
  probing_ = true;
  startedMs_ = millis();
  state->bleProbe = BleProbePhase::Connecting;
  state->bleProbeAtMs = startedMs_;
  state->uiDirty = true;
  logLine("BLE", "Probe one-shot %s", target.id);

  if (!gProbeClient->connect(addr, true, true, false)) {
    finishProbe(state, BleProbePhase::Refused);
    return true;
  }
  return true;
}

bool BleScannerService::startScan(AppState* state) {
  if (!ready_ || busy_ || probing_ || state == nullptr) {
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
  if (state == nullptr) {
    return;
  }

  const bool peer = gPeerOn;
  if (state->blePeerOn != peer || state->bleBeaconOn != !probing_) {
    state->uiDirty = true;
  }
  state->blePeerOn = peer;
  state->bleBeaconOn = !probing_;
  copyText(state->blePeerId, sizeof(state->blePeerId), gPeerId);
  if (!probing_ && !peer) {
    startBeacon();
  }

  if (probing_) {
    if (gProbeEvent == ProbeEvent::Ok) {
      finishProbe(state, BleProbePhase::Ok);
      return;
    }
    if (gProbeEvent == ProbeEvent::Fail) {
      finishProbe(state, BleProbePhase::Refused);
      return;
    }
    if ((millis() - startedMs_) > (AppConfig::BLE_PROBE_TIMEOUT_MS + 800)) {
      if (gProbeClient != nullptr) {
        gProbeClient->cancelConnect();
      }
      finishProbe(state, BleProbePhase::Timeout);
    }
    return;
  }

  if (!busy_) {
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

  for (uint8_t i = 0; i < gScratch.count; ++i) {
    const int idx = findBleIndex(*state, gScratch.items[i].id);
    if (idx >= 0) {
      mergeAdvert(&state->ble[static_cast<uint8_t>(idx)], gScratch.items[i]);
    } else if (state->bleShown < AppConfig::BLE_STORE_CAP) {
      state->ble[state->bleShown++] = gScratch.items[i];
    }
  }

  if (!isBleBrowseScreen(state->screen)) {
    sortByRssi(state->ble, state->bleShown);
  }

  state->bleTotal = state->bleShown;
  state->blePhase = gScratch.failed && state->bleShown == 0 ? ScanPhase::Failed
                                                            : ScanPhase::Complete;
  state->bleUpdatedMs = millis();
  if (!isBleBrowseScreen(state->screen)) {
    state->uiDirty = true;
  }
  busy_ = false;
  logLine("BLE", "Scan completed: %u devices", state->bleTotal);
}
