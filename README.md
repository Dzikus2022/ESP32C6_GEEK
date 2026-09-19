# ESP32-C6-GEEK

Osobny projekt PlatformIO na płytkę **Waveshare ESP32-C6-GEEK**.
Na razie tylko test USB Serial — bez GPIO, LCD, przycisków i czujników.

## Sprzęt

| Element | Wartość |
|---------|---------|
| Płytka | ESP32-C6-GEEK (Waveshare) |
| Chip | ESP32-C6 QFN40 |
| Flash | **16 MB** (onboard) |
| USB | native USB-Serial/JTAG |
| Port (Linux Mint) | `/dev/ttyACM0` |

Pinout peryferiów (LCD, TF, I2C, UART, przyciski) **nie jest jeszcze ustalony** w tym repo. Zrobimy to po udanym teście Serial.

## Środowisko

- **OS:** Linux Mint
- **IDE:** Cursor + PlatformIO
- **Framework:** Arduino (Espressif Arduino 3.x przez [pioarduino](https://github.com/pioarduino/platform-espressif32))
- **Definicja płytki PIO:** `esp32-c6-devkitc-1` (kompatybilna; nie ma gotowego `ESP32-C6-GEEK`)

Oficjalne `platform = espressif32` zostaje przy Arduino 2.x i **nie buduje** ESP32-C6 z `framework = arduino`. Dlatego używany jest pioarduino **54.03.20** (Arduino 3.2). Najnowszy 55.03.312 wymaga PlatformIO Core ≥ 6.2.0, a w tym systemie jest 6.1.19.

## Komendy

```bash
# budowa (nie kasuje flasha)
pio run

# wgranie — tylko zapis aplikacji, BEZ erase-flash
pio run -t upload

# monitor 115200
pio device monitor
```

Nie uruchamiaj `pio run -t erase` ani `esptool erase-flash`.

Jeśli `/dev/ttyACM0` jest niedostępny, dodaj użytkownika do grupy `dialout` i zaloguj się ponownie:

```bash
sudo usermod -aG dialout "$USER"
```
