# Rozwój

## Narzędzia

- Linux Mint, Cursor, PlatformIO Core 6.1.19
- Arduino 3.2 (pioarduino 54.03.20)
- Reguły: `.cursor/rules/00-project-core.mdc`

## Komendy

```bash
pio run
pio run -t upload
pio device monitor
```

Bez `erase` / `erase-flash`. Upload tylko na prośbę; po zmianie C/C++ obowiązkowe `pio run`.

Serial: prefiksy `[SYSTEM] [DISPLAY] [INPUT]`, nie co pętlę.

## Konwencje

- Czytaj kod i szukaj symboli zanim dodasz nowe
- GPIO tylko w `HardwareConfig.h` + [HARDWARE.md](HARDWARE.md)
- Serwisy nie rysują LCD
- Jeden `AppState`
- `ButtonManager` tylko zgłasza gesty
- `main.cpp` cienki
- Dokumentacja w tym samym zadaniu
- Nie commituj `.pio/`, binariów, sekretów

## Git

Remote: `https://github.com/Dzikus2022/ESP32C6_GEEK.git`.

| Gałąź | Rola |
|-------|------|
| `main` | stabilne aplikacje |
| `base` | fundament płyty (ten dokument) |
| `playground` | eksperymenty z `base` |

Eksperyment: `git checkout base && git checkout -b playground/...`. Nie mieszaj featurek do `base` bez potrzeby.
