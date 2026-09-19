# Konfiguracja

Źródło prawdy: `platformio.ini`. Nie ma jeszcze `include/config.h` ani flag funkcyjnych.

## PlatformIO (`[env:esp32-c6-geek]`)

| Opcja | Wartość | Znaczenie |
|-------|---------|-----------|
| `platform` | pioarduino `54.03.20` (URL zip) | Arduino 3.2 na ESP32-C6 |
| `board` | `esp32-c6-devkitc-1` | kompatybilna definicja C6 QFN40 |
| `framework` | `arduino` | wymagane przez projekt |
| `monitor_speed` | `115200` | USB Serial |
| `upload_port` | `/dev/ttyACM0` | Linux Mint, native USB |
| `monitor_port` | `/dev/ttyACM0` | j.w. |
| `board_upload.flash_size` | `16MB` | rozmiar dla esptool |
| `board_build.flash_size` | `16MB` | rozmiar budowania |
| `board_build.partitions` | `default_16MB.csv` | tabela 16 MB (Arduino) |

## Flag kompilacji

| Flaga | Po co |
|-------|--------|
| `ARDUINO_USB_MODE=1` | native USB (Hardware CDC / JTAG) |
| `ARDUINO_USB_CDC_ON_BOOT=1` | `Serial.println()` po USB CDC od startu |

To nie są przypisania GPIO peryferiów.

## Timing w firmware (na razie w `src/main.cpp`)

| Zachowanie | Wartość | Uwaga |
|------------|---------|--------|
| Baud Serial | 115200 | zgodne z `monitor_speed` |
| Opóźnienie po `Serial.begin` | ~2000 ms | tylko start |
| Okres komunikatu alive | ~1000 ms | `delay()` w `loop()` — tymczasowe, do wymiany gdy pojawi się logika runtime |

## Sieć / MQTT / sekrety

Brak. Projekt nie ma Wi‑Fi, MQTT ani tokenów.

`.gitignore` już ignoruje przyszłe `include/config.h` oraz `.env`. **Nie commituj** haseł, tokenów ani kluczy. Nie wklejaj ich do dokumentacji.
