# ESP32C6_GEEK

**GEEK Radar v0.2.0** — dashboard radia + **WiFi Sniffer v0.1** (802.11 → USB → Wireshark) na **Waveshare ESP32-C6-GEEK**.

| | |
|---|---|
| Chip | ESP32-C6 QFN40 (rev 0.2) |
| Flash | 16 MB |
| LCD | ST7789 240×135 |
| Framework | Arduino 3.x (pioarduino) |
| IDE / OS | Cursor + PlatformIO na Linux Mint |
| USB | native USB-Serial/JTAG → `/dev/ttyACM0` |

Jeden przycisk BOOT, nawigacja hierarchiczna: krótki = dalej, długi = wejdź / otwórz, bardzo długi = wstecz.  
Mapa pinów: [docs/HARDWARE.md](docs/HARDWARE.md). Architektura: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Ekrany

1. Splash — GEEK RADAR, status SYSTEM / DISPLAY / RADIO
2. SYSTEM — uptime, heap, liczby Wi‑Fi/BLE
3. WIFI — podsumowanie sieci; Long → lista AP → szczegóły
4. BLE — podsumowanie urządzeń; Long → lista urządzeń → szczegóły reklam
5. WIFI SNIFFER — kanał 6, statystyki; Long → strumień binarny na USB

Listy pokazują pozycję `n/total` i jedną wyróżnioną pozycję. Kolejność BLE jest zamrażana przy wejściu (sort RSSI raz); wybór trzyma adres, nie indeks.

Skan AP/BLE jest bierny. Sniffer 802.11 też jest bierny (bez deautha). Mostek: [docs/WIRESHARK.md](docs/WIRESHARK.md).

**TF/SD nie jest używane.** Waveshare V1 i V2 mają inne okablowanie karty — omijamy konflikt.

## Repozytorium

```
src/main.cpp          # setup / loop
src/core/             # App, stan, nawigacja, log
src/config/           # piny i stałe
src/display/          # LCD + ekrany
src/input/            # przycisk BOOT
src/services/         # system, Wi-Fi scan/sniff, BLE
tools/                # mostek Wireshark
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
| [docs/WIRESHARK.md](docs/WIRESHARK.md) | Sniffer USB, FIFO, protokół GKW1 |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Moduły, stan, nawigacja |
| [docs/HARDWARE.md](docs/HARDWARE.md) | LCD, BOOT, V1/V2 SD |
| [docs/CONFIGURATION.md](docs/CONFIGURATION.md) | Timing, pojemności, lib_deps |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | Narzędzia, Git |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | dialout, CDC, LCD, przycisk |
| [docs/CHANGELOG.md](docs/CHANGELOG.md) | Istotne zmiany |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Decyzje inżynierskie |

`docs/PROTOCOLS.md` nie istnieje — brak MQTT/HTTP API.

Remote: https://github.com/Dzikus2022/ESP32C6_GEEK.git
