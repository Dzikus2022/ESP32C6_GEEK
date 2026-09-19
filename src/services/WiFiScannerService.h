#pragma once

#include "core/AppState.h"

class WiFiScannerService {
 public:
  void begin();
  bool startScan(AppState* state);
  void tick(AppState* state);
  bool isBusy() const;

 private:
  uint32_t startedMs_ = 0;
  bool busy_ = false;

  void applyResults(AppState* state);
};
