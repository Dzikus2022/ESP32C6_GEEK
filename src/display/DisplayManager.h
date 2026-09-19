#pragma once

#include <Arduino.h>

class DisplayManager {
 public:
  bool begin();
  bool isReady() const { return ready_; }

  uint16_t width() const;
  uint16_t height() const;

  void fillBackground();
  void drawHeader(const char* title, const char* right);
  void drawText(int16_t x, int16_t y, const char* text, uint16_t color,
                uint8_t size = 1);
  void drawTextOn(int16_t x, int16_t y, const char* text, uint16_t fg,
                  uint16_t bg, uint8_t size = 1);
  void drawSignalBar(int16_t x, int16_t y, int8_t rssi);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

 private:
  bool ready_ = false;
};
