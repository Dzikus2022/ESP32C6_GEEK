# GEEK WiFi Sniffer → Wireshark

Bierny monitor 802.11 na ESP32-C6-GEEK. Bez deautha, bez wstrzykiwania, bez deszyfracji WPA.

```
ESP32-C6 promiscuous (kanał stały)
        ↓
kolejka ramek + USB binary (CAPTURE MODE)
        ↓
tools/geek_sniffer.py
        ↓
Radiotap + PCAP (FIFO)
        ↓
wireshark -k -i /tmp/geek-wifi
```

## Ograniczenia v0.1

- Tylko **2.4 GHz**.
- **Stały kanał** — domyślnie **6** (`WIFI_SNIFF_CHANNEL` w `AppConfig.h`). Brak hoppingu.
- LCD pokazuje statystyki; Wireshark jest na PC.
- Ruch **zaszyfrowany** (WPA2/3) w Wiresharku zostaje szyfrogramem. Sniffer nie ma kluczy i ich nie zgaduje.
- Native USB `/dev/ttyACM0` wymaga grupy `dialout`.
- W CAPTURE MODE firmware **nie** pisze tekstu na Serial — tylko ramki binarne.

## Wejście na LCD

1. Krótki BOOT: SYSTEM → WIFI → BLE → **WIFI SNIFFER**.
2. Na `WIFI SNIFFER` monitor już liczy pakiety (IDLE).
3. **LONG** (~700 ms) — start/stop strumienia USB (`LIVE` / `IDLE`).
4. **HOLD** (~1.8 s) — wstecz do dashboardu WIFI (strumień się wyłącza).

Nie startuje sam po bootcie.

## Mostek Linux

```bash
# pyserial: apt install python3-serial   albo   pip w venv
# albo interpreter PlatformIO, który już ma pyserial:
# ~/.platformio/penv/bin/python tools/geek_sniffer.py ...
pip3 install -r tools/requirements.txt
python3 tools/geek_sniffer.py \
    --port /dev/ttyACM0 \
    --fifo /tmp/geek-wifi \
    --verbose
```

Opcje: `--baud 115200`, `--output capture.pcap`.

FIFO `/tmp/geek-wifi` powstaje sam. Diagnostyka idzie na **stderr**.

Każdy nowy czytelnik FIFO dostaje **świeży** 24-bajtowy nagłówek PCAP (`d4 c3 b2 a1`). Jeśli Wireshark się zamknie, mostek czeka i przy kolejnym otwarciu znowu pisze nagłówek — nie kontynuuje w połowie strumienia.

Tylko **jeden** czytelnik FIFO naraz. Stary `dd` / drugi mostek / drugi Wireshark psuje magię.

```bash
wireshark -k -i /tmp/geek-wifi
```

Najpierw mostek (czeka na readera FIFO), potem Wireshark.

Self-test nagłówka (bez płytki):

```bash
python3 tools/geek_sniffer.py --self-test
python3 tools/test_geek_sniffer.py
```

Link type: **127** `LINKTYPE_IEEE802_11_RADIOTAP`.
Oczekiwany stos: Radiotap → IEEE 802.11 (Beacon, Probe, Control, Data).

## Protokół USB (little-endian)

| Pole | Typ | Opis |
|------|-----|------|
| MAGIC | 4 × u8 | `47 4B 57 31` (`GKW1`) |
| VERSION | u8 | `1` |
| SEQUENCE | u16 | numer kolejny |
| TIMESTAMP_US | u32 | `esp_timer` µs (zawija) |
| CHANNEL | u8 | kanał 1–14 |
| RSSI | i8 | dBm |
| ORIGINAL_LENGTH | u16 | długość MPDU z radia |
| CAPTURE_LENGTH | u16 | bajty w `FRAME_DATA` (≤ 256) |
| FRAME_DATA | u8[] | surowy IEEE 802.11 |
| CRC32 | u32 | IEEE nad VERSION…FRAME_DATA |

Parser szuka MAGIC, sprawdza długości i CRC, po śmieciach resynchronizuje się od następnego MAGIC. Granice `read()` ≠ granice ramek.

Radiotap jest minimalny: **Flags**, **Channel** (MHz + flaga 2 GHz) i **dBm Antsignal**. Bez noise/MCS/rate.

ESP32-C6 `sig_len` obejmuje 4-bajtowy FCS. Mostek **zostawia** te 4 bajty i ustawia `IEEE80211_RADIOTAP_F_FCS` (`0x10`) w Flags, gdy `CAPTURE_LENGTH == ORIGINAL_LENGTH`. Przy obcięciu do 256 B flagi FCS nie ma — ostatnie bajty nie są FCS. Bez tej flagi Wireshark traktuje FCS jako IE i oznacza Beacon jako *Malformed Packet*.

## Diagnostyka

- `Couldn't run dumpcap ... Permission denied`: `/usr/bin/dumpcap` jest `root:wireshark` (750). `sudo usermod -aG wireshark "$USER"`, potem nowe logowanie. Albo `sg wireshark` jeśli grupa już jest.
- `Permission denied` na ACM0: `dialout`, nowa sesja / `sg dialout`.
- Mostek cisza, LCD `PKT/s 0`: zły kanał (nie 6) albo monitor Wi‑Fi wyłączony w okolicy.
- LCD `DROP` rośnie: USB/PC nie nadąża — OK, callback nigdy nie czeka.
- Wireshark „Ethernet”: zły link type — używaj tego mostka, nie surowego Serial.
- `pio device monitor` w trakcie CAPTURE zepsuje PCAP (zajęty port / tekst). Nie mieszaj.

Nie uruchamiaj `erase-flash`.
