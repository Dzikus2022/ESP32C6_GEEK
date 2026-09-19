# ESP32C6_GEEK

Projekt PlatformIO na płytkę **Waveshare ESP32-C6-GEEK**.
Obecnie: test **USB Serial** — bez sterowania GPIO, LCD, TF ani czujnikami.

| | |
|---|---|
| Chip | ESP32-C6 QFN40 |
| Flash | 16 MB |
| Framework | Arduino 3.x (pioarduino) |
| IDE / OS | Cursor + PlatformIO na Linux Mint |
| USB | native USB-Serial/JTAG → `/dev/ttyACM0` |

Szczegóły sprzętu (w tym brak mapy pinów): [docs/HARDWARE.md](docs/HARDWARE.md).

## Repozytorium

```
src/main.cpp              # setup / loop — test Serial
platformio.ini            # budowa, port, 16 MB, USB CDC
docs/                     # dokumentacja szczegółowa
.cursor/rules/            # stałe reguły Cursor
```

## Budowa i Serial

```bash
pio run                 # budowa (nie kasuje flasha)
pio run -t upload       # wgranie na /dev/ttyACM0
pio device monitor      # 115200
```

Nie uruchamiaj `pio run -t erase` ani `esptool erase-flash`.
Komendy, Core PIO i konwencje: [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).
Port zajęty / brak uprawnień: [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

## Dokumentacja

| Plik | Treść |
|------|--------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Drzewo źródeł, boot, loop |
| [docs/HARDWARE.md](docs/HARDWARE.md) | Potwierdzony sprzęt; GPIO TBD |
| [docs/CONFIGURATION.md](docs/CONFIGURATION.md) | `platformio.ini`, flagi, timing |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | Narzędzia, Git, workflow |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | dialout, pioarduino, CDC |
| [docs/CHANGELOG.md](docs/CHANGELOG.md) | Istotne zmiany |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Świadome decyzje inżynierskie |

`docs/PROTOCOLS.md` nie istnieje — brak MQTT/HTTP/API.

Remote: https://github.com/Dzikus2022/ESP32C6_GEEK.git
