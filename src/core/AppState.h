#pragma once

#include <Arduino.h>

enum class ScreenId : uint8_t {
  Splash = 0,
  Main,
};

enum class LastButton : uint8_t {
  None = 0,
  Short,
  Long,
  VeryLong,
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

  bool displayOk = false;
  bool buttonReady = false;
  uint16_t pressCount = 0;
  LastButton lastButton = LastButton::None;
  uint32_t flashUntilMs = 0;

  bool uiDirty = true;
  uint32_t lastDashboardMs = 0;
  uint32_t lastHeartbeatMs = 0;
};
