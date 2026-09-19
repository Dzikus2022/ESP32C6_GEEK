#pragma once

#include "core/AppState.h"
#include "display/DisplayManager.h"
#include "input/ButtonManager.h"
#include "services/BleScannerService.h"
#include "services/SystemInfoService.h"
#include "services/WiFiScannerService.h"

class App {
 public:
  void begin();
  void tick();

 private:
  AppState state_{};
  uint32_t bootMs_ = 0;

  DisplayManager display_{};
  ButtonManager button_{};
  SystemInfoService systemInfo_{};
  WiFiScannerService wifi_{};
  BleScannerService ble_{};

  enum class PendingScan : uint8_t { None, Wifi, Ble, Both };

  PendingScan pendingScan_ = PendingScan::None;

  void handleButton(ButtonEvent event);
  void nextScreen();
  void requestRefresh();
  void serviceScans();
  void maybeAutoScan();
  bool radioIdle() const;
  void renderIfNeeded();
  void heartbeat();
};
