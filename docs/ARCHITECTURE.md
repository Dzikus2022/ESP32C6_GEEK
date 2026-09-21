# Architektura

ESP32-C6-GEEK **Base Firmware** `0.1-base`. Warstwy: **serwisy → `AppState` → Display**. Serwisy nie rysują LCD. `ButtonManager` tylko zgłasza gesty.

## Drzewo

```
src/
  main.cpp                 # setup / loop
  config/
    HardwareConfig.h       # potwierdzone GPIO
    AppConfig.h            # wersja, timing, kolory
  core/
    App.*                  # orchestracja
    AppState.h             # jedyny model stanu
    Log.*                  # [SYSTEM] [DISPLAY] [INPUT]
  display/
    DisplayManager.*       # ST7789, nagłówek, tekst z tłem
    Screens.*              # splash + test przycisku
  input/
    ButtonManager.*        # Short / Long / VeryLong / Repeat / Released
  services/
    SystemInfoService.*    # chip, flash, heap, uptime
```

`main.cpp` zostaje entry pointem (~11 linii).

## Właściciel stanu

`App` trzyma jeden `AppState`. `SystemInfoService` zapisuje snapshot. Display tylko czyta.

| `ScreenId` | Treść |
|------------|--------|
| `Splash` | BASE FIRMWARE, status SYSTEM / DISPLAY / BUTTON |
| `Main` | test przycisku + informacje o chipie |

Brak nawigacji między aplikacjami. Brak Wi‑Fi / BLE / sniffera.

## Przycisk

Zdarzenia pochodzą wyłącznie z `ButtonManager`. Ekrany nie mierzą GPIO.

| Zdarzenie | Działanie na `Main` |
|-----------|---------------------|
| ShortPress | `pressCount++`, `LAST: SHORT` |
| LongPress | krótki komunikat `LONG PRESS` |
| VeryLongPress | krótki komunikat `VERY LONG` |
| Repeat / Released | ignorowane w bazie |

## Inicjalizacja

1. USB Serial 115200 + ~2 s
2. snapshot systemu
3. LCD + podświetlenie
4. BOOT `INPUT_PULLUP`
5. splash (`millis`, bez długich `delay`)
6. ekran testu przycisku

**Nie** wywołujemy `WiFi.begin`, NimBLE, Zigbee, Thread ani MQTT.

## Runtime

`loop` → `App::tick()`: przycisk, splash, odświeżenie systemu, rysowanie, heartbeat Serial co 10 s.

Pełne `fillScreen` przy zmianie ekranu / gestu. Heap i uptime idą live (tekst z tłem).

## Świadomie niezaimplementowane

Sieć, MQTT, HA, web, Zigbee/Thread, TF/SD, skan Wi‑Fi/BLE, promiscuous, PCAP. To domena gałęzi odchodzących od `base`.
