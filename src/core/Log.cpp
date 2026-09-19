#include "core/Log.h"

#include <Arduino.h>
#include <cstdarg>
#include <cstdio>

namespace {
bool gCaptureStream = false;
}

void setCaptureStreamActive(bool active) { gCaptureStream = active; }

bool isCaptureStreamActive() { return gCaptureStream; }

void logLine(const char* tag, const char* fmt, ...) {
  if (gCaptureStream) {
    return;
  }
  char body[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(body, sizeof(body), fmt, args);
  va_end(args);
  Serial.printf("[%s] %s\n", tag, body);
}
