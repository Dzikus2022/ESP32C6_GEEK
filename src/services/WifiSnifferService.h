#pragma once

#include "core/AppState.h"

class WifiSnifferService {
 public:
  bool start(AppState* state);
  void stop(AppState* state);
  void tick(AppState* state);
  bool isRunning() const { return running_; }
  bool isStreaming() const { return streaming_; }
  void setStreaming(bool on, AppState* state);

 private:
  bool running_ = false;
  bool streaming_ = false;
  uint16_t seq_ = 0;
  uint32_t streamed_ = 0;
  uint32_t lastPpsMs_ = 0;
  uint32_t lastReceived_ = 0;

  void publishStats(AppState* state);
  void streamQueued();
};
