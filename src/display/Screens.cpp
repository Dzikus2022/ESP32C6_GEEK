#include "display/Screens.h"

#include <cstdio>

#include "config/AppConfig.h"

namespace {

const char* lastButtonLabel(LastButton id) {
  switch (id) {
    case LastButton::Short:
      return "SHORT";
    case LastButton::Long:
      return "LONG";
    case LastButton::VeryLong:
      return "VERY LONG";
    case LastButton::None:
    default:
      return "--";
  }
}

void drawSplash(DisplayManager& display, const AppState& state, bool full) {
  if (full) {
    display.fillBackground();
  }
  display.drawText(12, 16, AppConfig::APP_NAME, AppConfig::COLOR_ACCENT, 2);
  display.drawText(12, 42, "BASE FIRMWARE", AppConfig::COLOR_TEXT, 1);

  if (state.splashStep >= 1) {
    display.drawText(12, 68, "SYSTEM     OK", AppConfig::COLOR_OK, 1);
  }
  if (state.splashStep >= 2) {
    display.drawText(12, 84,
                     state.displayOk ? "DISPLAY    OK" : "DISPLAY    FAIL",
                     state.displayOk ? AppConfig::COLOR_OK : AppConfig::COLOR_WARN,
                     1);
  }
  if (state.splashStep >= 3) {
    display.drawText(12, 100,
                     state.buttonReady ? "BUTTON     READY" : "BUTTON     --",
                     state.buttonReady ? AppConfig::COLOR_OK
                                       : AppConfig::COLOR_MUTED,
                     1);
  }
  display.drawText(12, 120, AppConfig::APP_VERSION, AppConfig::COLOR_MUTED, 1);
}

void drawMain(DisplayManager& display, const AppState& state, bool full) {
  if (full) {
    display.fillBackground();
  }
  display.drawHeader("ESP32-C6-GEEK BASE", AppConfig::APP_VERSION);

  display.drawText(8, 26, "BUTTON TEST", AppConfig::COLOR_ACCENT, 1);

  char line[40];
  snprintf(line, sizeof(line), "PRESS COUNT: %-5u", state.pressCount);
  display.drawText(8, 48, line, AppConfig::COLOR_TEXT, 1);

  const bool flashing = state.flashUntilMs != 0 && millis() < state.flashUntilMs;
  if (flashing && state.lastButton == LastButton::Long) {
    display.drawText(8, 68, "LAST: LONG PRESS   ", AppConfig::COLOR_WARN, 1);
  } else if (flashing && state.lastButton == LastButton::VeryLong) {
    display.drawText(8, 68, "LAST: VERY LONG    ", AppConfig::COLOR_WARN, 1);
  } else {
    snprintf(line, sizeof(line), "LAST: %-12s", lastButtonLabel(state.lastButton));
    display.drawText(8, 68, line, AppConfig::COLOR_TEXT, 1);
  }

  snprintf(line, sizeof(line), "%s rev %u", state.system.chipModel,
           state.system.chipRevision);
  display.drawText(8, 90, line, AppConfig::COLOR_MUTED, 1);
  snprintf(line, sizeof(line), "HEAP %lu  UP %lus",
           static_cast<unsigned long>(state.system.heapFree),
           static_cast<unsigned long>(state.system.uptimeSec));
  display.drawText(8, 106, line, AppConfig::COLOR_MUTED, 1);
}

}  // namespace

void drawCurrentScreen(DisplayManager& display, const AppState& state,
                       bool fullClear) {
  if (state.screen == ScreenId::Splash) {
    drawSplash(display, state, fullClear);
    return;
  }
  drawMain(display, state, fullClear);
}
