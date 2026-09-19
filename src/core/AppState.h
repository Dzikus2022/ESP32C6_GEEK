#pragma once

#include <Arduino.h>

#include "config/AppConfig.h"

enum class ScreenId : uint8_t {
  Splash = 0,
  Dashboard,
  WifiRadar,
  BleRadar,
};

constexpr uint8_t kScreenCount = 4;

enum class ScanPhase : uint8_t {
  Idle = 0,
  Running,
  Complete,
  Failed,
};

struct WifiNetwork {
  char ssid[33];
  int8_t rssi;
  uint8_t channel;
  uint8_t authMode;
};

struct BleAdvert {
  char name[24];
  char id[18];
  int8_t rssi;
};

struct SystemSnapshot {
  char chipModel[16];
  uint8_t chipRevision;
  uint32_t flashBytes;
  uint32_t heapFree;
  uint32_t uptimeSec;
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

  bool displayOk = false;
  bool radioReady = false;
  bool uiDirty = true;
  uint32_t lastDashboardMs = 0;
  uint32_t lastHeartbeatMs = 0;
};
