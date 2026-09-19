#pragma once

#include "core/AppState.h"

class BleScannerService {
 public:
  bool begin();
  bool startScan(AppState* state);
  bool startProbe(AppState* state, const BleAdvert& target);
  void tick(AppState* state);
  bool isBusy() const;
  void hush();
  void unhush();

 private:
  bool ready_ = false;
  bool busy_ = false;
  bool probing_ = false;
  uint32_t startedMs_ = 0;

  void stopScanIfRunning();
  void finishProbe(AppState* state, BleProbePhase result);
  void startBeacon();
  void stopBeacon();
};
