#pragma once

#include <Arduino.h>

#include "config/AppConfig.h"

constexpr uint8_t CAPTURE_MAGIC[4] = {0x47, 0x4B, 0x57, 0x31};
constexpr uint8_t CAPTURE_VERSION = 1;
constexpr uint16_t CAPTURE_HEADER_SIZE = 17;

struct SniffFrame {
  uint32_t timestampUs = 0;
  uint8_t channel = 0;
  int8_t rssi = 0;
  uint8_t frameType = 0;
  uint16_t origLen = 0;
  uint16_t captLen = 0;
  uint8_t data[AppConfig::WIFI_SNIFF_MAX_LEN]{};
};

uint32_t captureCrc32(const uint8_t* data, size_t len);
size_t captureEncode(uint8_t* out, size_t outLen, uint16_t seq,
                     const SniffFrame& frame);
