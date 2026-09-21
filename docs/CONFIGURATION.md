# Konfiguracja

Piny: [HARDWARE.md](HARDWARE.md) / `src/config/HardwareConfig.h`.  
Timing i UI: `src/config/AppConfig.h`.  
PlatformIO: `platformio.ini`.

## Aplikacja

| Stała | Wartość |
|-------|---------|
| Nazwa | ESP32-C6-GEEK |
| Wersja | 0.1-base |
| Serial | 115200, start ~2 s, heartbeat 10 s |
| Debounce BOOT | 40 ms |
| Long press | 700 ms (w trakcie trzymania, raz) |
| Repeat start | 1100 ms (zdarzenie jest, baza je ignoruje) |
| Repeat interval | 160 ms |
| Very long press | 1800 ms (w trakcie trzymania) |
| Krok splash | 280 ms |
| Odświeżenie live | 500 ms |
| Flash gestu | 1500 ms |

Brak haseł, tokenów, MQTT i stałych sieciowych.

## Przycisk — semantyka

`ButtonManager` emituje zdarzenia, nie zna aplikacji:

- ShortPress: zwolnienie zanim minie 700 ms
- LongPress: raz, w 700 ms trzymania
- Repeat: co 160 ms po 1100 ms (baza ignoruje)
- VeryLongPress: raz, w 1800 ms trzymania
- Released: puszczenie po Long (baza ignoruje)

## PlatformIO

pioarduino 54.03.20, `esp32-c6-devkitc-1`, Arduino, `/dev/ttyACM0`, 16 MB, `default_16MB.csv`, USB CDC.

`lib_deps`:

- `adafruit/Adafruit GFX Library`
- `adafruit/Adafruit ST7735 and ST7789 Library`

`lib_ignore`: `SD`, `Adafruit seesaw Library` (transytywne; TF wyłączone).

NimBLE i inne stosy radiowe **nie** są w bazie.

Flagi USB: `ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1`.
