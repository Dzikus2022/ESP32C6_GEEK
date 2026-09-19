# ESP32C6_GEEK

**GEEK Radar v0.1.0** — kompaktowy dashboard radia/środowiska na **Waveshare ESP32-C6-GEEK**.

| | |
|---|---|
| Chip | ESP32-C6 QFN40 (rev 0.2) |
| Flash | 16 MB |
| LCD | ST7789 240×135 |
| Framework | Arduino 3.x (pioarduino) |
| IDE / OS | Cursor + PlatformIO na Linux Mint |
| USB | native USB-Serial/JTAG → `/dev/ttyACM0` |

Krótki przycisk BOOT: następny ekran. Długi: ponowny skan bieżącej strony.  
Mapa pinów: [docs/HARDWARE.md](docs/HARDWARE.md). Architektura: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Ekrany

1. Splash — GEEK RADAR, status SYSTEM / DISPLAY / RADIO  
2. Dashboard — uptime, heap, liczby Wi‑Fi/BLE, chip  
3. Wi‑Fi Radar — bierne odkrywanie sieci (bez łączenia)  
4. BLE Radar — bierne odkrywanie reklam (bez łączenia)

**TF/SD nie jest używane.** Waveshare V1 i V2 mają inne okablowanie karty — omijamy konflikt.

## Repozytorium

```
src/main.cpp          # setup / loop
src/core/             # App, stan, log
src/config/           # piny i stałe
src/display/          # LCD + ekrany
src/input/            # przycisk BOOT
src/services/         # system, Wi-Fi scan, BLE scan
docs/
```

## Budowa i Serial

```bash
pio run                 # budowa (nie kasuje flasha)
pio run -t upload       # wgranie na /dev/ttyACM0
pio device monitor      # 115200
```

Nie uruchamiaj `pio run -t erase` ani `esptool erase-flash`.
Szczegóły: [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md), [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

## Dokumentacja

| Plik | Treść |
|------|--------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Moduły, stan, przepływ |
| [docs/HARDWARE.md](docs/HARDWARE.md) | LCD, BOOT, V1/V2 SD |
| [docs/CONFIGURATION.md](docs/CONFIGURATION.md) | Timing, flagi, lib_deps |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | Narzędzia, Git |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | dialout, CDC, LCD |
| [docs/CHANGELOG.md](docs/CHANGELOG.md) | Istotne zmiany |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Decyzje inżynierskie |

`docs/PROTOCOLS.md` nie istnieje — brak MQTT/HTTP API.

Remote: https://github.com/Dzikus2022/ESP32C6_GEEK.git
