#pragma once

#include "core/AppState.h"

class SystemInfoService {
 public:
  void refresh(SystemSnapshot* out, uint32_t bootMs);
};
