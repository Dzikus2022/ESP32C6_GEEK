#include <Arduino.h>

#include "core/App.h"

namespace {
App gApp;
}

void setup() { gApp.begin(); }

void loop() { gApp.tick(); }
