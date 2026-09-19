#include "core/App.h"

#include "config/AppConfig.h"
#include "core/Log.h"
#include "core/Navigation.h"
#include "display/Screens.h"

void App::begin() {
  bootMs_ = millis();
  Serial.begin(AppConfig::SERIAL_BAUD);
  delay(AppConfig::SERIAL_BOOT_DELAY_MS);

  logLine("SYSTEM", "GEEK Radar %s starting", AppConfig::APP_VERSION);
  systemInfo_.refresh(&state_.system, bootMs_);
  logLine("SYSTEM", "Chip %s rev %u flash %lu", state_.system.chipModel,
          state_.system.chipRevision,
          static_cast<unsigned long>(state_.system.flashBytes));

  state_.displayOk = display_.begin();
  button_.begin();
  wifi_.begin();
  state_.radioReady = ble_.begin();

  state_.screen = ScreenId::Splash;
  state_.splashStep = 0;
  state_.splashStepAtMs = millis();
  state_.uiDirty = true;
  logLine("SYSTEM", "Boot complete, splash");
}

bool App::radioIdle() const {
  return !wifi_.isBusy() && !ble_.isBusy() && !sniffer_.isRunning();
}

void App::requestRefresh() {
  if (state_.screen == ScreenId::MainWifi || state_.screen == ScreenId::WifiList) {
    pendingScan_ = PendingScan::Wifi;
    logLine("INPUT", "Refresh Wi-Fi");
  } else if (state_.screen == ScreenId::MainBle ||
             state_.screen == ScreenId::BleList) {
    pendingScan_ = PendingScan::Ble;
    logLine("INPUT", "Refresh BLE");
  } else if (state_.screen == ScreenId::MainSystem) {
    pendingScan_ = PendingScan::Both;
    logLine("INPUT", "Refresh radio");
  }
}

void App::lockHold() { holdLocked_ = true; }

void App::resetHoldFlags() {
  holdLocked_ = false;
  sawLongThisPress_ = false;
  sawRepeatThisPress_ = false;
  sawVeryLongThisPress_ = false;
}

void App::activateTopLevelHold() {
  if (state_.screen == ScreenId::MainSystem) {
    requestRefresh();
  } else {
    enterCurrentFeature(&state_);
  }
  lockHold();
}

void App::handleButton(ButtonEvent event) {
  if (event == ButtonEvent::None) {
    return;
  }
  if (!state_.splashDone) {
    state_.splashStep = AppConfig::SPLASH_STEP_COUNT;
    return;
  }

  if (event == ButtonEvent::Released) {
    if (!holdLocked_ && isListScreen(state_.screen) && sawLongThisPress_ &&
        !sawRepeatThisPress_ && !sawVeryLongThisPress_) {
      if (state_.browse.count == 0) {
        requestRefresh();
      } else {
        openSelectedDetails(&state_);
      }
    }
    resetHoldFlags();
    return;
  }

  if (holdLocked_) {
    return;
  }

  if (event == ButtonEvent::VeryLongPress) {
    sawVeryLongThisPress_ = true;
    if (state_.screen == ScreenId::WifiSniffer) {
      state_.screen = ScreenId::MainWifi;
      state_.uiDirty = true;
      lockHold();
    } else if (isListScreen(state_.screen) || isDetailScreen(state_.screen)) {
      goBack(&state_);
      lockHold();
    } else if (isTopLevelScreen(state_.screen)) {
      activateTopLevelHold();
    }
    return;
  }

  if (state_.screen == ScreenId::WifiSniffer) {
    if (event == ButtonEvent::ShortPress) {
      nextTopLevel(&state_);
    } else if (event == ButtonEvent::LongPress) {
      if (sniffer_.isRunning()) {
        sniffer_.setStreaming(!sniffer_.isStreaming(), &state_);
      }
      lockHold();
    }
    return;
  }

  if (isTopLevelScreen(state_.screen)) {
    if (event == ButtonEvent::ShortPress) {
      nextTopLevel(&state_);
      maybeAutoScan();
    } else if (event == ButtonEvent::LongPress || event == ButtonEvent::Repeat) {
      activateTopLevelHold();
    }
    return;
  }

  if (isListScreen(state_.screen)) {
    if (event == ButtonEvent::LongPress) {
      sawLongThisPress_ = true;
      if (state_.browse.count == 0) {
        requestRefresh();
        lockHold();
      }
      return;
    }
    if (state_.browse.count == 0) {
      return;
    }
    if (event == ButtonEvent::ShortPress) {
      browseNext(&state_);
    } else if (event == ButtonEvent::Repeat) {
      sawRepeatThisPress_ = true;
      browseNext(&state_);
    }
    return;
  }

  if (isDetailScreen(state_.screen)) {
    if (event == ButtonEvent::ShortPress) {
      const uint8_t pages = state_.screen == ScreenId::BleDetails
                                ? AppConfig::BLE_DETAIL_PAGES
                                : AppConfig::WIFI_DETAIL_PAGES;
      state_.browse.detailPage =
          static_cast<uint8_t>((state_.browse.detailPage + 1) % pages);
      state_.uiDirty = true;
    } else if (event == ButtonEvent::LongPress &&
               state_.screen == ScreenId::BleDetails) {
      const BleAdvert* row = selectedBle(state_);
      if (row != nullptr && state_.bleProbe != BleProbePhase::Connecting) {
        pendingScan_ = PendingScan::None;
        ble_.startProbe(&state_, *row);
        lockHold();
      }
    }
    return;
  }
}

void App::maybeAutoScan() {
  const uint32_t now = millis();
  if (state_.screen == ScreenId::MainWifi) {
    const bool stale = state_.wifiUpdatedMs == 0 ||
                       (now - state_.wifiUpdatedMs) > AppConfig::SCAN_STALE_MS;
    if (stale && state_.wifiPhase != ScanPhase::Running) {
      pendingScan_ = PendingScan::Wifi;
    }
  } else if (state_.screen == ScreenId::MainBle) {
    const bool stale = state_.bleUpdatedMs == 0 ||
                       (now - state_.bleUpdatedMs) > AppConfig::SCAN_STALE_MS;
    if (stale && state_.blePhase != ScanPhase::Running) {
      pendingScan_ = PendingScan::Ble;
    }
  }
}

void App::serviceScans() {
  if (!radioIdle() || pendingScan_ == PendingScan::None) {
    return;
  }

  if (pendingScan_ == PendingScan::Wifi || pendingScan_ == PendingScan::Both) {
    if (wifi_.startScan(&state_)) {
      pendingScan_ =
          (pendingScan_ == PendingScan::Both) ? PendingScan::Ble : PendingScan::None;
      state_.uiDirty = true;
      return;
    }
  }

  if (pendingScan_ == PendingScan::Ble) {
    if (ble_.startScan(&state_)) {
      pendingScan_ = PendingScan::None;
      state_.uiDirty = true;
    }
  }
}

void App::renderIfNeeded() {
  if (!display_.isReady()) {
    return;
  }

  const uint32_t now = millis();
  if (state_.screen != drawnScreen_ || state_.browse.index != drawnIndex_ ||
      state_.browse.detailPage != drawnPage_ ||
      state_.browse.count != drawnCount_) {
    state_.uiDirty = true;
  }

  bool live = false;
  if ((now - state_.lastDashboardMs) >= AppConfig::DASHBOARD_REDRAW_MS) {
    if (state_.screen == ScreenId::MainSystem ||
        state_.screen == ScreenId::MainWifi ||
        state_.screen == ScreenId::MainBle ||
        state_.screen == ScreenId::WifiSniffer || isListScreen(state_.screen) ||
        isDetailScreen(state_.screen)) {
      systemInfo_.refresh(&state_.system, bootMs_);
      if (state_.browse.locked) {
        resolveBrowseSelection(&state_);
      }
      state_.lastDashboardMs = now;
      live = true;
    }
  }

  if (state_.uiDirty) {
    drawCurrentScreen(display_, state_, true);
    state_.uiDirty = false;
    drawnScreen_ = state_.screen;
    drawnIndex_ = state_.browse.index;
    drawnPage_ = state_.browse.detailPage;
    drawnCount_ = state_.browse.count;
    state_.lastDashboardMs = now;
    return;
  }

  if (live) {
    drawCurrentScreen(display_, state_, false);
  }
}

void App::heartbeat() {
  if (isCaptureStreamActive()) {
    return;
  }
  const uint32_t now = millis();
  if ((now - state_.lastHeartbeatMs) < AppConfig::SERIAL_HEARTBEAT_MS) {
    return;
  }
  state_.lastHeartbeatMs = now;
  systemInfo_.refresh(&state_.system, bootMs_);
  logLine("SYSTEM", "up %lus heap %lu screen %u wifi %u ble %u sel %s",
          static_cast<unsigned long>(state_.system.uptimeSec),
          static_cast<unsigned long>(state_.system.heapFree),
          static_cast<unsigned>(state_.screen), state_.wifiTotal, state_.bleTotal,
          state_.browse.selectedKey);
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
        state_.screen = ScreenId::MainSystem;
        state_.uiDirty = true;
        pendingScan_ = PendingScan::Both;
        logLine("SYSTEM", "Dashboard");
      }
    }
  }

  if (state_.screen == ScreenId::WifiSniffer) {
    pendingScan_ = PendingScan::None;
    if (!sniffer_.isRunning()) {
      ble_.hush();
      sniffer_.start(&state_);
    }
  } else if (sniffer_.isRunning()) {
    sniffer_.stop(&state_);
    ble_.unhush();
  }

  wifi_.tick(&state_);
  sniffer_.tick(&state_);
  ble_.tick(&state_);
  if (state_.screen == ScreenId::BleList && state_.browse.count == 0 &&
      state_.bleShown > 0) {
    captureBleBrowse(&state_);
    state_.uiDirty = true;
  }
  if (state_.screen == ScreenId::WifiList && state_.browse.count == 0 &&
      state_.wifiShown > 0) {
    captureWifiBrowse(&state_);
    state_.uiDirty = true;
  }
  if (state_.browse.locked) {
    resolveBrowseSelection(&state_);
  }
  serviceScans();
  renderIfNeeded();
  heartbeat();
}
