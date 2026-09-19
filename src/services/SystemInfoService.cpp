#include "services/SystemInfoService.h"

#include <Arduino.h>
#include <cstring>

void SystemInfoService::refresh(SystemSnapshot* out, uint32_t bootMs) {
  if (out == nullptr) {
    return;
  }

  const char* model = ESP.getChipModel();
  strncpy(out->chipModel, model != nullptr ? model : "ESP32-C6",
          sizeof(out->chipModel) - 1);
  out->chipModel[sizeof(out->chipModel) - 1] = '\0';
  out->chipRevision = static_cast<uint8_t>(ESP.getChipRevision());
  out->flashBytes = ESP.getFlashChipSize();
  out->heapFree = ESP.getFreeHeap();
  out->uptimeSec = (millis() - bootMs) / 1000UL;
}
