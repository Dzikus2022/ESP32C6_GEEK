# Konfiguracja

Piny: [HARDWARE.md](HARDWARE.md) / `src/config/HardwareConfig.h`.  
Timing i UI: `src/config/AppConfig.h`.  
PlatformIO: `platformio.ini`.

## Aplikacja

| Stała | Wartość |
|-------|---------|
| Nazwa | GEEK RADAR |
| Wersja | 0.1.0 |
| Serial | 115200, start ~2 s, heartbeat 10 s |
| Debounce BOOT | 40 ms |
| Długi przycisk | 700 ms |
| Krok splash | 280 ms |
| Odświeżenie dashboardu | 500 ms |
| Skan „stary” | 20 s |
| Timeout Wi‑Fi | 8 s |
| Czas BLE | 3.5 s |
| Wyników w pamięci | 12 Wi‑Fi / 12 BLE |
| Wierszy na radarze | 5 |

Brak haseł Wi‑Fi, tokenów i MQTT. Skan jest bierny.

## PlatformIO

Bez zmian względem poprzedniego env: pioarduino 54.03.20, `esp32-c6-devkitc-1`, Arduino, `/dev/ttyACM0`, 16 MB, `default_16MB.csv`, USB CDC.

Nowe `lib_deps`:

- `adafruit/Adafruit GFX Library`
- `adafruit/Adafruit ST7735 and ST7789 Library`
- `h2zero/NimBLE-Arduino`

`lib_ignore`: `SD`, `Adafruit seesaw Library` (transytywne; TF wyłączone).

Flagi USB bez zmian: `ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1`.
