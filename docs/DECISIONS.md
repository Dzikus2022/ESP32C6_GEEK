# Decyzje

Świadome wybory, których nie należy odwracać bez zrozumienia powodu.

## Trzy gałęzie

- `main` — stabilne aplikacje (np. WiFi Sniffer).
- `base` — czysty fundament Waveshare ESP32-C6-GEEK (LCD, BOOT, PIO, 16 MB).
- `playground` — eksperymenty odgałęzione od `base`.

Nowa funkcja sieciowa nie wraca do `base`, dopóki nie jest uniwersalnym sterownikiem płyty.

## pioarduino zamiast oficjalnego `espressif32`

ESP32-C6 + `framework = arduino` wymaga Arduino 3.x. Oficjalna platforma PlatformIO zostaje przy Arduino 2.x i nie buduje C6. Dlatego `platform` wskazuje zip [pioarduino/platform-espressif32](https://github.com/pioarduino/platform-espressif32).

## Pin platformy 54.03.20

55.03.312 wymaga PlatformIO Core ≥ 6.2.0. Lokalne Core to 6.1.19. 54.03.20 daje Arduino 3.2 i buduje się bez podnoszenia Core. Nie podnoś platformy do 55.x bez aktualizacji Core (albo odwrotnie) — to para zależna.

## Definicja `esp32-c6-devkitc-1`

Nie ma oficjalnego board ID `ESP32-C6-GEEK`. DevKitC-1 odpowiada chipowi ESP32-C6 QFN40. To definicja **kompatybilna**, nie pełny opis peryferiów Waveshare. Nie kopiuj ślepo pinów DevKit do GEEK.

## Flash 16 MB i `default_16MB.csv`

Fizyczny NOR to 16 MB. Tabela `default_16MB.csv` jest standardową tabelą Arduino dla 16 MB. Log PIO „8MB Flash” pochodzi z JSON-a DevKitC-1 i nie może nadpisać tej decyzji.

## USB CDC w `build_flags`

Płytka używa native USB-Serial/JTAG, nie mostka UART. Bez `ARDUINO_USB_MODE=1` i `ARDUINO_USB_CDC_ON_BOOT=1` `Serial.println()` nie wychodzi na `/dev/ttyACM0`. To konfiguracja USB, nie mapa GPIO.

## LCD i BOOT z oficjalnych dem Waveshare

Piny LCD (GPIO 1/2/3/4/5/6) i BOOT (GPIO 9, active-low) pochodzą z `DEV_Config` oraz `05_LCD_Button` w repozytorium Waveshare — **identyczne dla V1 i V2**. Nie kopiować pinów z nieoficjalnych list (HA / „silent refresh”).

## Brak TF/SD w bazie

V2 zmienia okablowanie karty. Implementacja SD bez potwierdzonej rewizji płytki grozi konfliktem V1/V2. `lib_ignore` dla `SD`.

## Adafruit ST7789 zamiast TFT_eSPI

TFT_eSPI wymaga `User_Setup.h`. Adafruit przyjmuje piny w konstruktorze, rysuje bez pełnego framebuffera (240×135×2 ≈ 63 KB oszczędzone). Waveshare: `SPI_MODE3`.

## Baza bez radia

`base` nie inicjuje Wi‑Fi, BLE, Zigbee, Thread ani MQTT. Stosy radiowe dodaje się w gałęzi eksperymentu, gdy są potrzebne.

## Jeden właściciel stanu

`AppState` jest jeden. `ButtonManager` nie zna ekranów ani sieci.

## Dokumentacja bez PROTOCOLS.md

Baza nie ma API MQTT/HTTP — pusty PROTOCOLS.md nie powstaje.

## Brak kasowania flasha

`erase-flash` / `pio run -t erase` nie są częścią workflow. Upload zapisuje aplikację (i w razie potrzeby bootloader/partycje), ale nie czyści całego układu „na wszelki wypadek”.
