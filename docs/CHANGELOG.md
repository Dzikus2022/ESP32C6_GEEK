# Changelog

Bez numerów wersji, dopóki projekt nie wprowadzi formalnego versioningu. Najnowsze wpisy na górze.

## 2026-09-19 — Hierarchiczna nawigacja BLE/Wi‑Fi (v0.1.1)

- Jeden `ScreenId`: dashboard SYSTEM/WIFI/BLE, listy i szczegóły.
- BOOT: Short = dalej, Long = wejdź / otwórz, VeryLong = wstecz, Repeat = szybkie przewijanie listy.
- Lista BLE: snapshot posortowany raz po RSSI; wybór po adresie; merge reklam bez przestawiania kolejności.
- Szczegóły BLE z pól reklamy (bez zgadywania, bez łączenia).
- Ten sam model dla listy i szczegółów Wi‑Fi (BSSID, bez auto-connect).
- Pusta lista: komunikat + Long = rescan, VeryLong = wstecz.
- `ButtonManager`: VeryLong i Repeat; brak podwójnego Short+Long po tym samym geście.

## 2026-09-19 — GEEK Radar v0.1.0

- Dashboard LCD: splash, system, Wi‑Fi radar, BLE radar.
- BOOT: krótki = strona, długi = skan (debounce).
- Architektura: serwisy → `AppState` → UI; `main.cpp` tylko entry point.
- Piny LCD/BOOT z oficjalnych dem Waveshare V1/V2. TF/SD celowo pominięte.
- Zależności: Adafruit GFX/ST7789, NimBLE-Arduino. USB Serial zachowany.

## 2026-09-19

- Dodane stałe reguły dokumentacji w `.cursor/rules/00-project-core.mdc`.
- Utworzona struktura `docs/` (architektura, sprzęt, konfiguracja, rozwój, changelog, decyzje, diagnostyka).
- README sprowadzony do punktu wejścia z linkami do `docs/`.
- `docs/PROTOCOLS.md` **nie** utworzony — brak MQTT/HTTP/API.

## 2026-09-19 (wcześniej)

- Stałe reguły rozwoju Cursor (`.cursor/rules/00-project-core.mdc`, `alwaysApply: true`).
- Pierwszy commit: projekt PlatformIO, Arduino, test USB Serial, 16 MB flash, port `/dev/ttyACM0`.
- Remote: `https://github.com/Dzikus2022/ESP32C6_GEEK.git`, gałąź `main`.
