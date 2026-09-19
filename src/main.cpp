#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("================================");
  Serial.println("Firmware started");
  Serial.println("Board: ESP32-C6-GEEK");
  Serial.println("Framework: Arduino");
  Serial.println("Test: USB Serial only (no GPIO)");
  Serial.printf("Chip: %s rev %d\n", ESP.getChipModel(),
                ESP.getChipRevision());
  Serial.printf("CPU freq: %u MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("SDK: %s\n", ESP.getSdkVersion());
  Serial.println("================================");
}

void loop() {
  Serial.println("ESP32-C6-GEEK alive");
  delay(1000);
}
