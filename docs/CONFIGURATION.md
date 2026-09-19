# Konfiguracja

Piny: [HARDWARE.md](HARDWARE.md) / `src/config/HardwareConfig.h`.  
Timing i UI: `src/config/AppConfig.h`.  
PlatformIO: `platformio.ini`.

## Aplikacja

| Stała | Wartość |
|-------|---------|
| Nazwa | GEEK RADAR |
| Wersja | 0.1.1 |
| Serial | 115200, start ~2 s, heartbeat 10 s |
| Debounce BOOT | 40 ms |
| Long press | 700 ms (na zwolnieniu, przed Repeat) |
| Repeat start | 1100 ms (przytrzymanie, listy) |
| Repeat interval | 160 ms |
| Very long press | 1800 ms (wstecz, w trakcie trzymania) |
| Krok splash | 280 ms |
| Odświeżenie dashboardu | 500 ms |
| Skan „stary” | 20 s |
| Timeout Wi‑Fi | 8 s |
| Czas BLE | 3.5 s |
| Katalog Wi‑Fi | 24 sieci |
| Katalog BLE | 32 urządzenia |
| Snapshot listy | 32 pozycje |
| Strony szczegółów | 4 BLE / 2 Wi‑Fi |

Brak haseł Wi‑Fi, tokenów i MQTT. Skan jest bierny. Long w szczegółach **nie** łączy.

## Przycisk — semantyka

`ButtonManager` emituje jedno zdarzenie na gest:

- ShortPress: zwolnienie < 700 ms
- LongPress: zwolnienie 700–1100 ms
- Repeat: co 160 ms po 1100 ms, dopóki trzymane
- VeryLongPress: raz, w 1800 ms trzymania

Po Repeat albo VeryLong zwolnienie **nie** emituje Short/Long.

## PlatformIO

Bez zmian względem poprzedniego env: pioarduino 54.03.20, `esp32-c6-devkitc-1`, Arduino, `/dev/ttyACM0`, 16 MB, `default_16MB.csv`, USB CDC.

`lib_deps`:

- `adafruit/Adafruit GFX Library`
- `adafruit/Adafruit ST7735 and ST7789 Library`
- `h2zero/NimBLE-Arduino`

`lib_ignore`: `SD`, `Adafruit seesaw Library` (transytywne; TF wyłączone).

Flagi USB bez zmian: `ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1`.
