# Sprzęt

Tylko fakty potwierdzone w tym repozytorium albo przez użytkownika przy podłączeniu płytki. **GPIO nie są zgadywane.**

## Potwierdzone

| Element | Wartość | Źródło |
|---------|---------|--------|
| Płytka | Waveshare ESP32-C6-GEEK | użytkownik / produkt |
| Chip | ESP32-C6 QFN40 | detekcja przy podłączeniu |
| Rewizja chipu | v0.2 | detekcja przy podłączeniu |
| Flash | 16 MB (onboard) | detekcja + konfiguracja projektu |
| USB | native USB-Serial/JTAG | detekcja |
| Port (Linux Mint) | `/dev/ttyACM0` | detekcja |
| Definicja PlatformIO | `esp32-c6-devkitc-1` | brak gotowego ID `ESP32-C6-GEEK` |

Firmware **nie steruje żadnym GPIO**. Test używa wyłącznie USB Serial.

## Peryferia płytki — pinout niepotwierdzony

Producent opisuje m.in. wyświetlacz LCD 1.14", slot TF oraz nagłówki I2C / UART / GPIO. W tym repo **nie ma** potwierdzonej mapy pinów.

Nie wpisuj tutaj numerów GPIO, adresów I2C ani linii SPI/UART, dopóki nie pochodzą z:

- potwierdzonej dokumentacji sprzętowej użytej w projekcie,
- albo jawnej informacji od użytkownika.

Gdy mapa pinów zostanie ustalona, ten plik jest **jedynym** miejscem na tabelę GPIO. README tylko linkuje tutaj.

## Zasilanie

Nieudokumentowane. Nie zakładaj napięć, prądów ani sposobu zasilania poza tym, że płytka jest podłączona przez native USB.

## Ograniczenia (potwierdzone w projekcie)

- Oficjalna definicja `esp32-c6-devkitc-1` w logu PlatformIO pokazuje „8MB Flash”. To opis JSON-a płytki referencyjnej, nie pojemność GEEK. Fizyczny flash to **16 MB**; partycje: `default_16MB.csv`.
- Waveshare ma rewizje GEEK i GEEK V2 z inną elektroniką. Rewizja **płytki** nie jest jeszcze potwierdzona w tym repo (potwierdzona jest rewizja **chipu** v0.2).
