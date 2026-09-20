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
python3 -m py_compile tools/geek_sniffer.py
python3 tools/geek_sniffer.py --self-test
python3 tools/test_geek_sniffer.py
```

Wireshark: [WIRESHARK.md](WIRESHARK.md). `pyserial` w `tools/requirements.txt`.
Nie włączaj `pio device monitor` gdy leci CAPTURE MODE.

Bez `erase` / `erase-flash`. Upload tylko na prośbę; po zmianie C/C++ obowiązkowe `pio run`.

Serial: prefiksy `[SYSTEM] [DISPLAY] [WIFI] [BLE] [SNIFF] [INPUT]`, nie co pętlę. W CAPTURE MODE logi aplikacji są wyciszone.

## Konwencje

- Czytaj kod i szukaj symboli zanim dodasz nowe
- GPIO tylko w `HardwareConfig.h` + [HARDWARE.md](HARDWARE.md)
- Skanery nie rysują LCD
- Jeden `AppState` i jeden `ScreenId`
- Nawigacja tylko przez `ButtonManager` + `Navigation`
- Lista BLE: snapshot kolejności, wybór po adresie
- `main.cpp` cienki
- Dokumentacja w tym samym zadaniu
- Nie commituj `.pio/`, binariów, sekretów

## Git

Remote: `https://github.com/Dzikus2022/ESP32C6_GEEK.git`, gałąź `main`.
