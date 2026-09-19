# Architektura

GEEK Radar v0.1. Warstwy: **serwisy → `AppState` → UI**. Skanery nie rysują LCD.

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
    Log.*                  # [SYSTEM] [DISPLAY] [WIFI] [BLE] [INPUT]
  display/
    DisplayManager.*       # ST7789, nagłówek, paski RSSI
    Screens.*              # splash / dashboard / wifi / ble
  input/
    ButtonManager.*        # debounce, krótki / długi BOOT
  services/
    SystemInfoService.*
    WiFiScannerService.*   # async WiFi.scanNetworks
    BleScannerService.*    # NimBLE advertise scan
```

`main.cpp` zostaje entry pointem (~11 linii).

## Właściciel stanu

`App` trzyma jeden `AppState`. Serwisy zapisują wyniki skanu i fazę. Display tylko czyta.

Pola m.in.: `screen`, snapshot systemu, `wifiPhase` / wyniki / timestamp, `blePhase` / wyniki / timestamp, flagi splash i `uiDirty`.

Wi‑Fi i BLE **nie skanują jednocześnie** — kolejka `PendingScan` (radio C6).

## Inicjalizacja

1. USB Serial 115200 + ~2 s (start)  
2. snapshot systemu  
3. LCD + podświetlenie  
4. BOOT INPUT_PULLUP  
5. Wi‑Fi STA, bez łączenia  
6. NimBLE init  
7. splash (maszyna `millis`, bez długich `delay`)  
8. dashboard + kolejka skanu Wi‑Fi, potem BLE  

## Runtime

`loop` → `App::tick()`: przycisk, splash, `wifi.tick` / `ble.tick`, kolejka skanów, rysowanie gdy brudne, heartbeat Serial co 10 s.

Krótki BOOT: Dashboard → Wi‑Fi → BLE → Dashboard.  
Długi BOOT: rescan bieżącej strony (na dashboardzie: oba, sekwencyjnie).

## Świadomie niezaimplementowane

MQTT, HA, web, Zigbee/Thread, TF/SD, tryb połączenia Wi‑Fi, narzędzia GPIO/I2C/UART. Wyniki skanu są zwykłymi strukturami — da się je później wystawić bez przepisywania skanerów.
