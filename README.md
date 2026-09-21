# ESP32-C6-GEEK Base Firmware

Czysty punkt startowy dla **Waveshare ESP32-C6-GEEK**.

Gałąź `base` trzyma potwierdzoną konfigurację płyty (PlatformIO, 16 MB flash, ST7789, BOOT) i minimalną aplikację testową. Nie zawiera Radaru, Sniffera, BLE ani mostka Wireshark.

| | |
|---|---|
| Płytka | Waveshare ESP32-C6-GEEK |
| Chip | ESP32-C6 QFN40 |
| Flash | 16 MB |
| LCD | ST7789 240×135, SPI_MODE3 |
| Przycisk | BOOT GPIO9, active-low |
| Framework | Arduino 3.x (pioarduino 54.03.20) |
| USB | native USB-Serial/JTAG → `/dev/ttyACM0` |
| Wersja | 0.1-base |

Mapa pinów: [docs/HARDWARE.md](docs/HARDWARE.md). Architektura: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Co robi ten firmware

Po starcie LCD pokazuje:

```
ESP32-C6-GEEK
BASE FIRMWARE
SYSTEM     OK
DISPLAY    OK
BUTTON     READY
v0.1-base
```

Potem ekran testu przycisku: krótki = licznik, długi = `LONG PRESS`, bardzo długi = `VERY LONG`. Dodatkowo chip, heap i uptime.

**Nie inicjuje Wi‑Fi, BLE, Zigbee, Thread ani MQTT.**

**TF/SD nie jest używane.** V1 i V2 mają inne okablowanie karty.

## Workflow gałęzi

| Gałąź | Rola |
|-------|------|
| `main` | stabilne aplikacje (np. WiFi Sniffer) |
| `base` | czysty fundament płyty — start eksperymentów |
| `playground` | eksperymenty odgałęzione od `base` |

Nowa praca: odgałęź od `base`, nie konfiguruj LCD/BOOT od zera.

## Repozytorium

```
src/main.cpp          # setup / loop
src/core/             # App, stan, log
src/config/           # piny i stałe
src/display/          # LCD + ekrany bazy
src/input/            # przycisk BOOT
src/services/         # snapshot systemu
docs/
.cursor/rules/        # stałe reguły projektu
```

## Budowa

```bash
pio run                 # budowa (nie kasuje flasha)
pio run -t upload       # wgranie na /dev/ttyACM0
pio device monitor      # 115200
```

Nie uruchamiaj `pio run -t erase` ani `esptool erase-flash`.

## Dokumentacja

| Plik | Treść |
|------|--------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Moduły, stan, ekrany bazy |
| [docs/HARDWARE.md](docs/HARDWARE.md) | LCD, BOOT, V1/V2 SD |
| [docs/CONFIGURATION.md](docs/CONFIGURATION.md) | Timing, kolory, lib_deps |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | Narzędzia, Git, gałęzie |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | dialout, CDC, LCD, przycisk |
| [docs/CHANGELOG.md](docs/CHANGELOG.md) | Istotne zmiany |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Decyzje inżynierskie |

`docs/PROTOCOLS.md` nie istnieje — brak MQTT/HTTP API.

Remote: https://github.com/Dzikus2022/ESP32C6_GEEK.git
