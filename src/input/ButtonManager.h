#pragma once

#include <Arduino.h>

enum class ButtonEvent : uint8_t {
  None = 0,
  ShortPress,
  LongPress,
  VeryLongPress,
  Repeat,
  Released,
};

class ButtonManager {
 public:
  void begin();
  ButtonEvent poll();

 private:
  bool lastRaw_ = false;
  bool stablePressed_ = false;
  bool longFired_ = false;
  bool veryLongFired_ = false;
  bool repeatArmed_ = false;
  uint32_t lastChangeMs_ = 0;
  uint32_t pressStartMs_ = 0;
  uint32_t lastRepeatMs_ = 0;
};
