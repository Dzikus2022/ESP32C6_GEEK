#include "services/WifiSnifferService.h"

#include <WiFi.h>
#include <esp_timer.h>
#include <esp_wifi.h>

#include "core/Log.h"
#include "services/CaptureProtocol.h"

namespace {

struct SniffQueue {
  SniffFrame slots[AppConfig::WIFI_SNIFF_QUEUE];
  volatile uint8_t head = 0;
  volatile uint8_t tail = 0;
  uint32_t received = 0;
  uint32_t dropped = 0;
  uint32_t mgmt = 0;
  uint32_t ctrl = 0;
  uint32_t data = 0;
  int8_t rssi = 0;
};

SniffQueue gQ;

void IRAM_ATTR classify(wifi_promiscuous_pkt_type_t type) {
  switch (type) {
    case WIFI_PKT_MGMT:
      ++gQ.mgmt;
      break;
    case WIFI_PKT_CTRL:
      ++gQ.ctrl;
      break;
    case WIFI_PKT_DATA:
      ++gQ.data;
      break;
    default:
      break;
  }
}

void IRAM_ATTR onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (buf == nullptr) {
    return;
  }
  const wifi_promiscuous_pkt_t* pkt =
      static_cast<const wifi_promiscuous_pkt_t*>(buf);
  uint32_t orig = pkt->rx_ctrl.sig_len;
  if (orig < 10 || orig > 4095) {
    ++gQ.dropped;
    return;
  }

  const uint8_t next =
      static_cast<uint8_t>((gQ.head + 1) % AppConfig::WIFI_SNIFF_QUEUE);
  if (next == gQ.tail) {
    ++gQ.dropped;
    return;
  }

  SniffFrame& slot = gQ.slots[gQ.head];
  slot.timestampUs = static_cast<uint32_t>(esp_timer_get_time());
  slot.channel = static_cast<uint8_t>(pkt->rx_ctrl.channel);
  slot.rssi = static_cast<int8_t>(pkt->rx_ctrl.rssi);
  slot.frameType = static_cast<uint8_t>(type);
  slot.origLen = static_cast<uint16_t>(orig);
  uint16_t capt = static_cast<uint16_t>(orig);
  if (capt > AppConfig::WIFI_SNIFF_MAX_LEN) {
    capt = AppConfig::WIFI_SNIFF_MAX_LEN;
  }
  slot.captLen = capt;
  for (uint16_t i = 0; i < capt; ++i) {
    slot.data[i] = pkt->payload[i];
  }

  gQ.head = next;
  ++gQ.received;
  gQ.rssi = slot.rssi;
  classify(type);
}

}  // namespace

bool WifiSnifferService::start(AppState* state) {
  if (running_) {
    return true;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  delay(20);

  wifi_promiscuous_filter_t filter{};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT |
                       WIFI_PROMIS_FILTER_MASK_CTRL |
                       WIFI_PROMIS_FILTER_MASK_DATA;
  if (esp_wifi_set_promiscuous_filter(&filter) != ESP_OK) {
    logLine("SNIFF", "Filter failed");
    return false;
  }
  wifi_promiscuous_filter_t ctrlFilter{};
  ctrlFilter.filter_mask = WIFI_PROMIS_CTRL_FILTER_MASK_ALL;
  if (esp_wifi_set_promiscuous_ctrl_filter(&ctrlFilter) != ESP_OK) {
    logLine("SNIFF", "Ctrl filter failed");
    return false;
  }
  if (esp_wifi_set_promiscuous_rx_cb(&onPromiscuous) != ESP_OK) {
    logLine("SNIFF", "Callback failed");
    return false;
  }
  if (esp_wifi_set_channel(AppConfig::WIFI_SNIFF_CHANNEL,
                           WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    logLine("SNIFF", "Channel failed");
    return false;
  }
  if (esp_wifi_set_promiscuous(true) != ESP_OK) {
    logLine("SNIFF", "Promiscuous failed");
    return false;
  }

  gQ.head = 0;
  gQ.tail = 0;
  gQ.received = 0;
  gQ.dropped = 0;
  gQ.mgmt = 0;
  gQ.ctrl = 0;
  gQ.data = 0;
  gQ.rssi = 0;
  running_ = true;
  streaming_ = false;
  seq_ = 0;
  streamed_ = 0;
  lastPpsMs_ = millis();
  lastReceived_ = 0;
  if (state != nullptr) {
    state->wifiSniff = WifiSniffStats{};
    state->wifiSniff.channel = AppConfig::WIFI_SNIFF_CHANNEL;
    state->wifiSniff.running = true;
    state->uiDirty = true;
  }
  logLine("SNIFF", "Monitor CH %u", AppConfig::WIFI_SNIFF_CHANNEL);
  return true;
}

void WifiSnifferService::stop(AppState* state) {
  if (streaming_) {
    setStreaming(false, state);
  }
  if (!running_) {
    return;
  }
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  running_ = false;
  if (state != nullptr) {
    state->wifiSniff.running = false;
    state->wifiSniff.streaming = false;
    state->uiDirty = true;
  }
  logLine("SNIFF", "Stopped");
}

void WifiSnifferService::setStreaming(bool on, AppState* state) {
  if (on == streaming_) {
    return;
  }
  if (on) {
    Serial.flush();
    setCaptureStreamActive(true);
    streaming_ = true;
  } else {
    streaming_ = false;
    setCaptureStreamActive(false);
    logLine("SNIFF", "USB idle");
  }
  if (state != nullptr) {
    state->wifiSniff.streaming = streaming_;
    state->uiDirty = true;
  }
}

void WifiSnifferService::publishStats(AppState* state) {
  if (state == nullptr) {
    return;
  }
  state->wifiSniff.channel = AppConfig::WIFI_SNIFF_CHANNEL;
  state->wifiSniff.received = gQ.received;
  state->wifiSniff.dropped = gQ.dropped;
  state->wifiSniff.mgmt = gQ.mgmt;
  state->wifiSniff.ctrl = gQ.ctrl;
  state->wifiSniff.data = gQ.data;
  state->wifiSniff.rssi = gQ.rssi;
  state->wifiSniff.running = running_;
  state->wifiSniff.streaming = streaming_;

  const uint32_t now = millis();
  if ((now - lastPpsMs_) >= 1000) {
    const uint32_t rx = gQ.received;
    state->wifiSniff.pps = static_cast<uint16_t>(rx - lastReceived_);
    lastReceived_ = rx;
    lastPpsMs_ = now;
  }
}

void WifiSnifferService::streamQueued() {
  uint8_t packet[CAPTURE_HEADER_SIZE + AppConfig::WIFI_SNIFF_MAX_LEN + 4];
  while (gQ.tail != gQ.head) {
    SniffFrame frame = gQ.slots[gQ.tail];
    gQ.tail = static_cast<uint8_t>((gQ.tail + 1) % AppConfig::WIFI_SNIFF_QUEUE);
    if (!streaming_) {
      continue;
    }
    const size_t n = captureEncode(packet, sizeof(packet), seq_, frame);
    if (n == 0) {
      ++gQ.dropped;
      continue;
    }
    if (Serial.availableForWrite() < static_cast<int>(n)) {
      ++gQ.dropped;
      continue;
    }
    Serial.write(packet, n);
    ++seq_;
    if (streamed_ < 0xFFFFFFFFu) {
      ++streamed_;
    }
  }
}

void WifiSnifferService::tick(AppState* state) {
  if (!running_) {
    return;
  }
  streamQueued();
  publishStats(state);
  if (state != nullptr) {
    state->wifiSniff.streamed = streamed_;
  }
}
