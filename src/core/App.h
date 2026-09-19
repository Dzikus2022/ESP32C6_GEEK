#pragma once

#include "core/AppState.h"
#include "display/DisplayManager.h"
#include "input/ButtonManager.h"
#include "services/BleScannerService.h"
#include "services/SystemInfoService.h"
#include "services/WiFiScannerService.h"
#include "services/WifiSnifferService.h"

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
  WifiSnifferService sniffer_{};
  BleScannerService ble_{};

  enum class PendingScan : uint8_t { None, Wifi, Ble, Both };

  PendingScan pendingScan_ = PendingScan::None;
  ScreenId drawnScreen_ = ScreenId::Splash;
  uint8_t drawnIndex_ = 255;
  uint8_t drawnPage_ = 255;
  uint8_t drawnCount_ = 255;
  bool holdLocked_ = false;
  bool sawLongThisPress_ = false;
  bool sawRepeatThisPress_ = false;
  bool sawVeryLongThisPress_ = false;

  void handleButton(ButtonEvent event);
  void lockHold();
  void resetHoldFlags();
  void activateTopLevelHold();
  void requestRefresh();
  void serviceScans();
  void maybeAutoScan();
  bool radioIdle() const;
  void renderIfNeeded();
  void heartbeat();
};
