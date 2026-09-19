#include "display/DisplayManager.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <cstring>

#include "config/AppConfig.h"
#include "config/HardwareConfig.h"
#include "core/Log.h"

namespace {

Adafruit_ST7789 gTft(&SPI, HardwareConfig::LCD_CS_PIN,
                     HardwareConfig::LCD_DC_PIN, HardwareConfig::LCD_RST_PIN);

uint8_t barCount(int8_t rssi) {
  if (rssi >= -50) {
    return 5;
  }
  if (rssi >= -60) {
    return 4;
  }
  if (rssi >= -70) {
    return 3;
  }
  if (rssi >= -80) {
    return 2;
  }
  return 1;
}

}  // namespace

bool DisplayManager::begin() {
  pinMode(HardwareConfig::LCD_BL_PIN, OUTPUT);
  analogWrite(HardwareConfig::LCD_BL_PIN, 200);

  SPI.begin(HardwareConfig::LCD_SCLK_PIN, -1, HardwareConfig::LCD_MOSI_PIN,
            HardwareConfig::LCD_CS_PIN);
  gTft.init(HardwareConfig::LCD_NATIVE_WIDTH, HardwareConfig::LCD_NATIVE_HEIGHT,
            SPI_MODE3);
  gTft.setRotation(1);
  gTft.setTextWrap(false);
  fillBackground();
  ready_ = true;
  logLine("DISPLAY", "ST7789 ready %ux%u", width(), height());
  return true;
}

uint16_t DisplayManager::width() const { return gTft.width(); }

uint16_t DisplayManager::height() const { return gTft.height(); }

void DisplayManager::fillBackground() {
  gTft.fillScreen(AppConfig::COLOR_BG);
}

void DisplayManager::drawHeader(const char* title, const char* right) {
  gTft.fillRect(0, 0, gTft.width(), 18, AppConfig::COLOR_HEADER);
  gTft.setTextSize(1);
  gTft.setTextColor(AppConfig::COLOR_TEXT);
  gTft.setCursor(6, 5);
  gTft.print(title != nullptr ? title : "");
  if (right != nullptr && right[0] != '\0') {
    const int16_t x = static_cast<int16_t>(gTft.width() - (strlen(right) * 6) - 6);
    gTft.setCursor(x > 80 ? x : 80, 5);
    gTft.print(right);
  }
}

void DisplayManager::drawText(int16_t x, int16_t y, const char* text,
                             uint16_t color, uint8_t size) {
  gTft.setTextSize(size);
  gTft.setTextColor(color);
  gTft.setCursor(x, y);
  gTft.print(text != nullptr ? text : "");
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                             uint16_t color) {
  gTft.fillRect(x, y, w, h, color);
}

void DisplayManager::drawSignalBar(int16_t x, int16_t y, int8_t rssi) {
  const uint8_t filled = barCount(rssi);
  for (uint8_t i = 0; i < 5; ++i) {
    const int16_t h = static_cast<int16_t>(4 + i * 2);
    const int16_t by = static_cast<int16_t>(y + (12 - h));
    const uint16_t color =
        (i < filled) ? AppConfig::COLOR_BAR : AppConfig::COLOR_BAR_EMPTY;
    gTft.fillRect(x + static_cast<int16_t>(i * 7), by, 5, h, color);
  }
}
