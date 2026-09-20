# Changelog

Bez numerów wersji, dopóki projekt nie wprowadzi formalnego versioningu. Najnowsze wpisy na górze.

## 2026-09-20 — Radiotap Flags: FCS na końcu ramki

- `sig_len` z ESP32 zawiera 4-bajtowy FCS. Wireshark bez flagi Flags/FCS czytał go jako IE → Beacon *Malformed Packet*.
- Mostek zachowuje FCS i ustawia `IEEE80211_RADIOTAP_F_FCS`. Firmware bez zmian.

## 2026-09-20 — FIFO PCAP: nagłówek przy każdej sesji

- Mostek pisał global header PCAP tylko przy pierwszym czytelniku FIFO. Kolejny Wireshark widział `ts_sec` jako magię (`0x00000128`).
- Teraz każdy reader dostaje `d4 c3 b2 a1` + DLT 127 zanim padnie pierwszy rekord. `--self-test` to sprawdza.

## 2026-09-20 — GEEK WiFi Sniffer v0.1 (firmware 0.2.0)

- Promiscuous 802.11 na stałym kanale 6, kolejka, USB binary `GKW1`.
- Ekran WIFI SNIFFER: Long = LIVE/IDLE, Very long = wstecz. CAPTURE MODE bez tekstu na Serial.
- `tools/geek_sniffer.py` → Radiotap/PCAP na FIFO `/tmp/geek-wifi` dla Wireshark.
- Bez deautha, wstrzykiwania i deszyfracji.

## 2026-09-19 — Radar reklamuje się jako GEEK RADAR (v0.1.5)

- Płytka nadaje nazwę `GEEK RADAR` (connectable). Telefon widzi ją w nRF Connect / LightBlue, nie w Ustawieniach Bluetooth iPhone.
- Połączenie **z telefonu do radaru** pokazuje `PHONE CONNECTED` / `PEER` na LCD.
- To nie jest flood ani pukanie do cudzego telefonu.

## 2026-09-19 — Jedna próba połączenia BLE (v0.1.4)

- Long w BLE Details wysyła **jeden** CONNECT do wybranego adresu, potem rozłącza.
- Wynik na LCD: `PROBE ...` / `OK` / `REFUSED` / `TIMEOUT`. Timeout 6 s, zero retry.
- Skan zatrzymuje się na czas próby. Brak pętli i floodu.

## 2026-09-19 — Stabilniejszy LCD i nazwy BLE (v0.1.3)

- Odświeżanie nie czyści całego ST7789 — znika skok obrazu przy RSSI/uptime.
- Lista BLE pokazuje nazwę (reklama, potem typ/vendor z appearance/manufacturer), nie sam MAC.
- MAC zostaje w szczegółach. Lista ma 3 wiersze naraz.

## 2026-09-19 — Long press otwiera listę od razu (v0.1.2)

- LongPress emituje się w 700 ms **trzymania**, nie dopiero po puszczeniu w oknie 700–1100 ms.
- Wejście w listę WIFI/BLE blokuje resztę tego samego gestu (Repeat/VeryLong nie wracają od razu).
- Szczegóły z listy: puszczenie po Long, jeśli nie było Repeat.

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
