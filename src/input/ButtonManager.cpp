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
    veryLongFired_ = false;
    repeatArmed_ = false;
    lastRepeatMs_ = now;
    return ButtonEvent::None;
  }

  if (rawPressed && stablePressed_) {
    const uint32_t held = now - pressStartMs_;
    if (!longFired_ && held >= AppConfig::BUTTON_LONG_PRESS_MS) {
      longFired_ = true;
      return ButtonEvent::LongPress;
    }
    if (!veryLongFired_ && held >= AppConfig::BUTTON_VERY_LONG_PRESS_MS) {
      veryLongFired_ = true;
      return ButtonEvent::VeryLongPress;
    }
    if (longFired_ && !veryLongFired_ &&
        held >= AppConfig::BUTTON_REPEAT_START_MS) {
      if ((now - lastRepeatMs_) >= AppConfig::BUTTON_REPEAT_INTERVAL_MS) {
        lastRepeatMs_ = now;
        repeatArmed_ = true;
        return ButtonEvent::Repeat;
      }
    }
  }

  if (!rawPressed && stablePressed_) {
    stablePressed_ = false;
    const uint32_t held = now - pressStartMs_;
    const bool hadLong = longFired_;
    longFired_ = false;
    veryLongFired_ = false;
    repeatArmed_ = false;
    if (!hadLong && held >= AppConfig::BUTTON_DEBOUNCE_MS) {
      return ButtonEvent::ShortPress;
    }
    if (hadLong) {
      return ButtonEvent::Released;
    }
  }

  return ButtonEvent::None;
}
