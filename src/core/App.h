#pragma once

#include "core/AppState.h"
#include "display/DisplayManager.h"
#include "input/ButtonManager.h"
#include "services/SystemInfoService.h"

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

  void handleButton(ButtonEvent event);
  void renderIfNeeded();
  void heartbeat();
};
