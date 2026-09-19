# Decyzje

Świadome wybory, których nie należy odwracać bez zrozumienia powodu.

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

## Brak TF/SD w v0.1

V2 zmienia okablowanie karty. Implementacja SD bez potwierdzonej rewizji płytki grozi konfliktem V1/V2. `lib_ignore` dla `SD`.

## NimBLE zamiast Bluedroid

Arduino 3.x ma BLE w rdzeniu (ciężki stos). NimBLE-Arduino oficjalnie wspiera ESP32-C6, zajmuje mniej RAM i skanuje same reklamy. Nie łączymy się z urządzeniami.

## Adafruit ST7789 zamiast TFT_eSPI

TFT_eSPI wymaga `User_Setup.h`. Adafruit przyjmuje piny w konstruktorze, rysuje bez pełnego framebuffera (240×135×2 ≈ 63 KB oszczędzone). Waveshare: `SPI_MODE3`.

## Jeden właściciel stanu i kolejka radia

`AppState` jest jeden. Wi‑Fi i BLE nie startują równolegle — współdzielenie radia na C6.

## Hierarchiczna nawigacja jednym przyciskiem

BOOT jest jedynym wejściem. Short/Long/VeryLong/Repeat są sematyką `ButtonManager`, nie timingiem w ekranach. Long na dashboardzie WIFI/BLE **wchodzi** w listę (nie rescan). Rescan zostaje na SYSTEM (Long) oraz na pustej liście (Long).

## Stabilny snapshot listy, nie indeks

Reklamy BLE i RSSI zmieniają się w tle. UI nie sortuje katalogu podczas `BleList`/`BleDetails`. Wybór to adres z `browse.selectedKey`. Dzięki temu użytkownik nie „przeskakuje” na inne urządzenie, gdy RSSI się zmieni.

## Brak auto-connect

Szczegóły są bierne. Long w `BleDetails`/`WifiDetails` nie łączy — nie ma jeszcze bezpiecznej, kompletnej ścieżki połączenia.

## Dokumentacja bez PROTOCOLS.md

Skan jest lokalny. MQTT/HTTP API nie istnieje — pusty PROTOCOLS.md nie powstaje.

## Brak kasowania flasha

`erase-flash` / `pio run -t erase` nie są częścią workflow. Upload zapisuje aplikację (i w razie potrzeby bootloader/partycje), ale nie czyści całego układu „na wszelki wypadek”.

Pusty `PROTOCOLS.md` nie powstaje, dopóki nie będzie MQTT/HTTP API.
