#include "core/App.h"

#include "config/AppConfig.h"
#include "core/Log.h"
#include "display/Screens.h"

void App::begin() {
  bootMs_ = millis();
  Serial.begin(AppConfig::SERIAL_BAUD);
  delay(AppConfig::SERIAL_BOOT_DELAY_MS);

  logLine("SYSTEM", "%s %s starting", AppConfig::APP_NAME, AppConfig::APP_VERSION);
  systemInfo_.refresh(&state_.system, bootMs_);
  logLine("SYSTEM", "Chip %s rev %u flash %lu", state_.system.chipModel,
          state_.system.chipRevision,
          static_cast<unsigned long>(state_.system.flashBytes));

  state_.displayOk = display_.begin();
  button_.begin();
  state_.buttonReady = true;

  state_.screen = ScreenId::Splash;
  state_.splashStep = 0;
  state_.splashStepAtMs = millis();
  state_.uiDirty = true;
  logLine("SYSTEM", "Base firmware ready");
}

void App::handleButton(ButtonEvent event) {
  if (event == ButtonEvent::None) {
    return;
  }
  if (!state_.splashDone) {
    state_.splashStep = AppConfig::SPLASH_STEP_COUNT;
    return;
  }

  if (event == ButtonEvent::ShortPress) {
    if (state_.pressCount < 0xFFFFu) {
      ++state_.pressCount;
    }
    state_.lastButton = LastButton::Short;
    state_.flashUntilMs = 0;
    state_.uiDirty = true;
    logLine("INPUT", "Short %u", state_.pressCount);
    return;
  }

  if (event == ButtonEvent::LongPress) {
    state_.lastButton = LastButton::Long;
    state_.flashUntilMs = millis() + AppConfig::BUTTON_FLASH_MS;
    state_.uiDirty = true;
    logLine("INPUT", "Long");
    return;
  }

  if (event == ButtonEvent::VeryLongPress) {
    state_.lastButton = LastButton::VeryLong;
    state_.flashUntilMs = millis() + AppConfig::BUTTON_FLASH_MS;
    state_.uiDirty = true;
    logLine("INPUT", "Very long");
  }
}

void App::renderIfNeeded() {
  if (!display_.isReady()) {
    return;
  }

  const uint32_t now = millis();
  bool live = false;
  if ((now - state_.lastDashboardMs) >= AppConfig::DASHBOARD_REDRAW_MS) {
    systemInfo_.refresh(&state_.system, bootMs_);
    state_.lastDashboardMs = now;
    live = state_.splashDone;
  }

  if (state_.flashUntilMs != 0 && now >= state_.flashUntilMs) {
    state_.flashUntilMs = 0;
    state_.uiDirty = true;
  }

  if (state_.uiDirty) {
    drawCurrentScreen(display_, state_, true);
    state_.uiDirty = false;
    state_.lastDashboardMs = now;
    return;
  }

  if (live) {
    drawCurrentScreen(display_, state_, false);
  }
}

void App::heartbeat() {
  const uint32_t now = millis();
  if ((now - state_.lastHeartbeatMs) < AppConfig::SERIAL_HEARTBEAT_MS) {
    return;
  }
  state_.lastHeartbeatMs = now;
  systemInfo_.refresh(&state_.system, bootMs_);
  logLine("SYSTEM", "up %lus heap %lu presses %u",
          static_cast<unsigned long>(state_.system.uptimeSec),
          static_cast<unsigned long>(state_.system.heapFree), state_.pressCount);
}

void App::tick() {
  handleButton(button_.poll());

  if (!state_.splashDone) {
    if ((millis() - state_.splashStepAtMs) >= AppConfig::SPLASH_STEP_MS) {
      state_.splashStepAtMs = millis();
      if (state_.splashStep < AppConfig::SPLASH_STEP_COUNT) {
        ++state_.splashStep;
        state_.uiDirty = true;
      } else {
        state_.splashDone = true;
        state_.screen = ScreenId::Main;
        state_.uiDirty = true;
        logLine("SYSTEM", "Button test");
      }
    }
  }

  renderIfNeeded();
  heartbeat();
}
