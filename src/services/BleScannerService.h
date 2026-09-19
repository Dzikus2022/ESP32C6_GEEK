#pragma once

#include "core/AppState.h"

class BleScannerService {
 public:
  bool begin();
  bool startScan(AppState* state);
  void tick(AppState* state);
  bool isBusy() const;

 private:
  bool ready_ = false;
  bool busy_ = false;
  uint32_t startedMs_ = 0;
};
