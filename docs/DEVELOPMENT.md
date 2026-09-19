# Rozwój

## Narzędzia

- **OS:** Linux Mint
- **IDE:** Cursor + PlatformIO
- **CLI:** `~/.platformio/penv/bin/pio` (lub `pio` w PATH)
- **PlatformIO Core:** 6.1.19 (lokalnie potwierdzone)
- **Framework:** Arduino 3.2 (pioarduino 54.03.20)
- **Reguły AI:** `.cursor/rules/00-project-core.mdc` (`alwaysApply: true`)

Oficjalne `platform = espressif32` (Arduino 2.x) **nie buduje** ESP32-C6. Szczegóły: [DECISIONS.md](DECISIONS.md).

## Komendy

```bash
# budowa — nie kasuje flasha
pio run

# wgranie aplikacji (bez erase-flash)
pio run -t upload

# monitor 115200 na /dev/ttyACM0
pio device monitor
```

Nie uruchamiaj `pio run -t erase` ani `esptool erase-flash`, chyba że użytkownik tego wyraźnie zażąda i zatwierdzi.

Budowa i wgranie to **osobne** czynności. Po zmianie C/C++ obowiązkowe jest `pio run`. Upload tylko na prośbę.

## Port szeregowy

Port: `/dev/ttyACM0` (grupa `dialout`). Jeśli `Permission denied`, zobacz [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

## Git

- Remote: `https://github.com/Dzikus2022/ESP32C6_GEEK.git`
- Gałąź: `main`
- Nie commituj `.pio/`, `*.bin`, `*.elf`, sekretów
- Commit i push tylko na prośbę użytkownika
- Kod i powiązana dokumentacja zwykle w tym samym commicie

## Konwencje

- Czytaj istniejący kod zanim dodasz nowy
- Szukaj symboli przed dodaniem funkcji / stałej / GPIO
- Nie zgaduj pinów — mapa tylko w [HARDWARE.md](HARDWARE.md)
- `main.cpp` zostaje cienkim entry pointem
- Dokumentacja jest częścią zadania (patrz reguła projektu)
