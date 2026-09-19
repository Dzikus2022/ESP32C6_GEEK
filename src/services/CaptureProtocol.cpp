#include "services/CaptureProtocol.h"

#include <cstring>

uint32_t captureCrc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      const uint32_t mask = static_cast<uint32_t>(-(static_cast<int32_t>(crc & 1u)));
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return crc ^ 0xFFFFFFFFu;
}

namespace {

void putU16(uint8_t* p, uint16_t v) {
  p[0] = static_cast<uint8_t>(v);
  p[1] = static_cast<uint8_t>(v >> 8);
}

void putU32(uint8_t* p, uint32_t v) {
  p[0] = static_cast<uint8_t>(v);
  p[1] = static_cast<uint8_t>(v >> 8);
  p[2] = static_cast<uint8_t>(v >> 16);
  p[3] = static_cast<uint8_t>(v >> 24);
}

}  // namespace

size_t captureEncode(uint8_t* out, size_t outLen, uint16_t seq,
                     const SniffFrame& frame) {
  if (out == nullptr || frame.captLen > AppConfig::WIFI_SNIFF_MAX_LEN) {
    return 0;
  }
  const size_t total =
      static_cast<size_t>(CAPTURE_HEADER_SIZE) + frame.captLen + 4u;
  if (outLen < total) {
    return 0;
  }

  out[0] = CAPTURE_MAGIC[0];
  out[1] = CAPTURE_MAGIC[1];
  out[2] = CAPTURE_MAGIC[2];
  out[3] = CAPTURE_MAGIC[3];
  out[4] = CAPTURE_VERSION;
  putU16(out + 5, seq);
  putU32(out + 7, frame.timestampUs);
  out[11] = frame.channel;
  out[12] = static_cast<uint8_t>(frame.rssi);
  putU16(out + 13, frame.origLen);
  putU16(out + 15, frame.captLen);
  if (frame.captLen > 0) {
    memcpy(out + CAPTURE_HEADER_SIZE, frame.data, frame.captLen);
  }
  const uint32_t crc =
      captureCrc32(out + 4, static_cast<size_t>(CAPTURE_HEADER_SIZE - 4) +
                                frame.captLen);
  putU32(out + CAPTURE_HEADER_SIZE + frame.captLen, crc);
  return total;
}
