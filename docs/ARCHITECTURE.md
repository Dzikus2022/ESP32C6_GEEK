# Architektura

GEEK Radar v0.1.1. Warstwy: **serwisy → `AppState` / modele → nawigacja → Display**. Skanery nie rysują LCD.

## Drzewo

```
src/
  main.cpp                 # setup / loop
  config/
    HardwareConfig.h       # potwierdzone GPIO
    AppConfig.h            # wersja, timing, pojemności, kolory
  core/
    App.*                  # orchestracja i obsługa zdarzeń przycisku
    AppState.h             # jedyny model stanu + ScreenId
    Navigation.*           # hierarchia ekranów, snapshot listy
    Log.*                  # [SYSTEM] [DISPLAY] [WIFI] [BLE] [INPUT]
  display/
    DisplayManager.*       # ST7789, nagłówek, paski RSSI
    Screens.*              # rysowanie aktualnego ScreenId
  input/
    ButtonManager.*        # Short / Long / VeryLong / Repeat
  services/
    SystemInfoService.*
    WiFiScannerService.*   # async WiFi.scanNetworks
    BleScannerService.*    # NimBLE advertise scan + merge katalogu
```

`main.cpp` zostaje entry pointem (~11 linii).

## Właściciel stanu

`App` trzyma jeden `AppState`. Serwisy zapisują wyniki skanu i fazę. `Navigation` zmienia tylko `screen` i `browse`. Display tylko czyta.

Właścicielem ekranu jest **jeden** `ScreenId`:

| Id | Poziom |
|----|--------|
| `Splash` | start |
| `MainSystem` | dashboard |
| `MainWifi` | dashboard Wi‑Fi |
| `MainBle` | dashboard BLE |
| `WifiList` | lista AP |
| `WifiDetails` | szczegóły AP |
| `BleList` | lista urządzeń |
| `BleDetails` | szczegóły urządzenia |

Nie ma osobnych flag typu `inBleList` / `showBleDetails`.

Wi‑Fi i BLE **nie skanują jednocześnie** — kolejka `PendingScan` (radio C6).

## Nawigacja jednym przyciskiem

Zdarzenia pochodzą wyłącznie z `ButtonManager`. Ekrany nie mierzą GPIO.

### Dashboard (`MainSystem` / `MainWifi` / `MainBle`)

| Zdarzenie | Działanie |
|-----------|-----------|
| ShortPress | następny ekran: SYSTEM → WIFI → BLE → SYSTEM |
| LongPress | na SYSTEM: rescan; na WIFI/BLE: **wejście** w listę |
| VeryLongPress | brak (już najwyższy poziom) |
| Repeat | ignorowane |

### Lista (`WifiList` / `BleList`)

| Zdarzenie | Działanie |
|-----------|-----------|
| ShortPress | następna pozycja |
| Repeat | szybkie przewijanie (gdy lista niepusta) |
| LongPress | otwórz szczegóły wybranej pozycji; pusta lista = rescan |
| VeryLongPress | powrót do dashboardu WIFI / BLE |

### Szczegóły (`WifiDetails` / `BleDetails`)

| Zdarzenie | Działanie |
|-----------|-----------|
| ShortPress | następna strona informacji |
| LongPress | brak (nie ma jeszcze bezpiecznej akcji połączenia) |
| VeryLongPress | powrót do listy **z tym samym** wybranym urządzeniem/siecią |
| Repeat | ignorowane |

## Stabilna lista BLE

Katalog skanera (`state.ble[]`) jest niezależny od UI.

Przy wejściu w `BleList` `captureBleBrowse()` tworzy **snapshot kolejności**:

1. bierze aktualnie znane urządzenia (max `BROWSE_CAP`),
2. sortuje snapshot **raz** po RSSI malejąco,
3. zapisuje adresy BLE w `browse.keys[]`,
4. ustawia `browse.locked = true`.

Dopóki użytkownik jest na `BleList` / `BleDetails`:

- katalog **nie jest przestawiany** po RSSI,
- nowe reklamy **scalają** istniejący wpis po adresie (RSSI, last-seen, pola AD),
- wybór trzyma **adres**, nie indeks tablicy katalogu.

`selectedBle()` zawsze wyszukuje po `browse.selectedKey`. UI nie trzyma wskaźnika do wiersza skanera między klatkami.

Zniknięcie urządzenia nie psuje nawigacji: pozycja zostaje, lista/szczegóły pokazują „brak w ostatnim skanie”.

To samo dla Wi‑Fi: klucz to BSSID (albo SSID, gdy BSSID puste), `captureWifiBrowse()`, merge po kluczu, bez sortowania podczas browse.

## Dane BLE w szczegółach

Tylko to, co przyszło z reklamy (albo `--` gdy brak flagi):

nazwa, adres, typ adresu, RSSI, wygładzony RSSI, last seen, liczba reklam, connectable, TX power, manufacturer ID/data, UUID usług, service data, appearance.

Firmware **nie łączy** się z urządzeniem i nie zgaduje brakujących pól.

## Pojemności

Stałe tablice, bez nieograniczonego wzrostu:

- katalog BLE: `BLE_STORE_CAP` = 32
- katalog Wi‑Fi: `WIFI_STORE_CAP` = 24
- snapshot browse: `BROWSE_CAP` = 32

## Inicjalizacja

1. USB Serial 115200 + ~2 s (start)
2. snapshot systemu
3. LCD + podświetlenie
4. BOOT INPUT_PULLUP
5. Wi‑Fi STA, bez łączenia
6. NimBLE init
7. splash (maszyna `millis`, bez długich `delay`)
8. `MainSystem` + kolejka skanu Wi‑Fi, potem BLE

## Runtime

`loop` → `App::tick()`: przycisk, splash, `wifi.tick` / `ble.tick`, `resolveBrowseSelection` gdy lista zablokowana, kolejka skanów, rysowanie gdy brudne, heartbeat Serial co 10 s.

## Świadomie niezaimplementowane

MQTT, HA, web, Zigbee/Thread, TF/SD, łączenie z AP / urządzeniem BLE, narzędzia GPIO/I2C/UART. Wyniki skanu są zwykłymi strukturami — da się je później wystawić bez przepisywania skanerów.
