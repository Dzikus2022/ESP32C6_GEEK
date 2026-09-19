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

## Brak mapy GPIO w kodzie i dokumentacji

Pinout LCD / TF / I2C / UART / przycisków **nie jest potwierdzony** w repo. Zgadywanie pinów jest zabronione. Tabela GPIO powstanie tylko w [HARDWARE.md](HARDWARE.md), gdy będzie źródło.

## Brak kasowania flasha

`erase-flash` / `pio run -t erase` nie są częścią workflow. Upload zapisuje aplikację (i w razie potrzeby bootloader/partycje), ale nie czyści całego układu „na wszelki wypadek”.

## Dokumentacja bez PROTOCOLS.md

Na razie nie ma zewnętrznego protokołu (MQTT, HTTP API, komendy sieciowe). Plik pojawi się, gdy taki interfejs powstanie — nie tworzyć pustego szablonu.
