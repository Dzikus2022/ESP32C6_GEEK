#include "input/ButtonManager.h"

#include "config/AppConfig.h"
#include "config/HardwareConfig.h"

void ButtonManager::begin() {
  pinMode(HardwareConfig::BOOT_BUTTON_PIN, INPUT_PULLUP);
  lastRaw_ = digitalRead(HardwareConfig::BOOT_BUTTON_PIN) == LOW;
  stablePressed_ = lastRaw_;
  lastChangeMs_ = millis();
}

ButtonEvent ButtonManager::poll() {
  const bool rawPressed =
      HardwareConfig::BOOT_BUTTON_ACTIVE_LOW
          ? digitalRead(HardwareConfig::BOOT_BUTTON_PIN) == LOW
          : digitalRead(HardwareConfig::BOOT_BUTTON_PIN) == HIGH;
  const uint32_t now = millis();

  if (rawPressed != lastRaw_) {
    lastRaw_ = rawPressed;
    lastChangeMs_ = now;
    return ButtonEvent::None;
  }

  if ((now - lastChangeMs_) < AppConfig::BUTTON_DEBOUNCE_MS) {
    return ButtonEvent::None;
  }

  if (rawPressed && !stablePressed_) {
    stablePressed_ = true;
    pressStartMs_ = now;
    longFired_ = false;
    return ButtonEvent::None;
  }

  if (rawPressed && stablePressed_ && !longFired_) {
    if ((now - pressStartMs_) >= AppConfig::BUTTON_LONG_PRESS_MS) {
      longFired_ = true;
      return ButtonEvent::LongPress;
    }
  }

  if (!rawPressed && stablePressed_) {
    stablePressed_ = false;
    if (!longFired_ && (now - pressStartMs_) >= AppConfig::BUTTON_DEBOUNCE_MS) {
      return ButtonEvent::ShortPress;
    }
  }

  return ButtonEvent::None;
}
