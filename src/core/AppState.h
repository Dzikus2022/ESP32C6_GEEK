#pragma once

#include <Arduino.h>
#include <cstring>

#include "config/AppConfig.h"

enum class ScreenId : uint8_t {
  Splash = 0,
  MainSystem,
  MainWifi,
  MainBle,
  WifiList,
  WifiDetails,
  BleList,
  BleDetails,
};

enum class ScanPhase : uint8_t {
  Idle = 0,
  Running,
  Complete,
  Failed,
};

struct WifiNetwork {
  char ssid[33];
  char bssid[18];
  int8_t rssi;
  uint8_t channel;
  uint8_t authMode;
};

constexpr uint8_t BLE_FLAG_CONNECTABLE = 1 << 0;
constexpr uint8_t BLE_FLAG_HAS_TX = 1 << 1;
constexpr uint8_t BLE_FLAG_HAS_APPEAR = 1 << 2;
constexpr uint8_t BLE_FLAG_HAS_MFG = 1 << 3;
constexpr uint8_t BLE_FLAG_HAS_UUID = 1 << 4;
constexpr uint8_t BLE_FLAG_HAS_SVC_DATA = 1 << 5;

struct BleAdvert {
  char name[24];
  char id[18];
  int8_t rssi;
  int8_t rssiSmooth;
  uint8_t addrType;
  uint8_t flags;
  int8_t txPower;
  uint16_t appearance;
  uint16_t manufacturerId;
  uint8_t mfgLen;
  uint8_t mfgData[6];
  uint8_t uuidCount;
  char uuid0[21];
  char uuid1[21];
  char svcDataUuid[21];
  uint8_t svcDataLen;
  uint8_t svcData[6];
  uint32_t lastSeenMs;
  uint16_t advCount;
};

struct SystemSnapshot {
  char chipModel[16];
  uint8_t chipRevision;
  uint32_t flashBytes;
  uint32_t heapFree;
  uint32_t uptimeSec;
};

struct BrowseNav {
  char keys[AppConfig::BROWSE_CAP][18];
  uint8_t count = 0;
  uint8_t index = 0;
  char selectedKey[18] = {};
  uint8_t detailPage = 0;
  bool locked = false;
};

struct AppState {
  ScreenId screen = ScreenId::Splash;
  uint8_t splashStep = 0;
  uint32_t splashStepAtMs = 0;
  bool splashDone = false;

  SystemSnapshot system{};

  ScanPhase wifiPhase = ScanPhase::Idle;
  uint16_t wifiTotal = 0;
  uint8_t wifiShown = 0;
  WifiNetwork wifi[AppConfig::WIFI_STORE_CAP]{};
  uint32_t wifiUpdatedMs = 0;

  ScanPhase blePhase = ScanPhase::Idle;
  uint16_t bleTotal = 0;
  uint8_t bleShown = 0;
  BleAdvert ble[AppConfig::BLE_STORE_CAP]{};
  uint32_t bleUpdatedMs = 0;

  BrowseNav browse{};

  bool displayOk = false;
  bool radioReady = false;
  bool uiDirty = true;
  uint32_t lastDashboardMs = 0;
  uint32_t lastHeartbeatMs = 0;
};

inline bool isTopLevelScreen(ScreenId id) {
  return id == ScreenId::MainSystem || id == ScreenId::MainWifi ||
         id == ScreenId::MainBle;
}

inline bool isListScreen(ScreenId id) {
  return id == ScreenId::WifiList || id == ScreenId::BleList;
}

inline bool isDetailScreen(ScreenId id) {
  return id == ScreenId::WifiDetails || id == ScreenId::BleDetails;
}

inline bool isBleBrowseScreen(ScreenId id) {
  return id == ScreenId::BleList || id == ScreenId::BleDetails;
}

inline bool isWifiBrowseScreen(ScreenId id) {
  return id == ScreenId::WifiList || id == ScreenId::WifiDetails;
}

inline int findBleIndex(const AppState& state, const char* id) {
  if (id == nullptr || id[0] == '\0') {
    return -1;
  }
  for (uint8_t i = 0; i < state.bleShown; ++i) {
    if (strncmp(state.ble[i].id, id, sizeof(state.ble[i].id)) == 0) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

inline int findWifiIndex(const AppState& state, const char* key) {
  if (key == nullptr || key[0] == '\0') {
    return -1;
  }
  for (uint8_t i = 0; i < state.wifiShown; ++i) {
    const char* id =
        state.wifi[i].bssid[0] != '\0' ? state.wifi[i].bssid : state.wifi[i].ssid;
    if (strncmp(id, key, 18) == 0) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

inline const BleAdvert* selectedBle(const AppState& state) {
  const int idx = findBleIndex(state, state.browse.selectedKey);
  if (idx < 0) {
    return nullptr;
  }
  return &state.ble[static_cast<uint8_t>(idx)];
}

inline const WifiNetwork* selectedWifi(const AppState& state) {
  const int idx = findWifiIndex(state, state.browse.selectedKey);
  if (idx < 0) {
    return nullptr;
  }
  return &state.wifi[static_cast<uint8_t>(idx)];
}
