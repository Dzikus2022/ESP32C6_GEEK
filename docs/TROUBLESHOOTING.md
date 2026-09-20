# Diagnostyka

Tylko problemy już napotkane przy stawianiu projektu.

## Brak uprawnień do `/dev/ttyACM0`

**Objaw:** `Could not open /dev/ttyACM0` / `Permission denied`.  
**Przyczyna:** urządzenie `root:dialout` `660`, użytkownik poza grupą `dialout`.  
**Rozpoznanie:** `ls -l /dev/ttyACM0` oraz `id` / `groups`.  
**Rozwiązanie (trwałe):** `sudo usermod -aG dialout "$USER"`, potem wylogowanie i ponowne logowanie.  
**Rozwiązanie (jednorazowe):** `sudo chmod 666 /dev/ttyACM0` (znika po odłączeniu USB).

## pioarduino 55.x nie instaluje się na PlatformIO 6.1.19

**Objaw:** `IncompatiblePlatform` — platforma wymaga Core ≥ 6.2.0.  
**Przyczyna:** pin 55.03.312 jest nowszy niż lokalne Core 6.1.19.  
**Rozwiązanie w tym projekcie:** `platform` = pioarduino **54.03.20**. Nie aktualizuj Core „przy okazji”.

## Oficjalne `espressif32` + Arduino na C6

**Objaw:** płytka „nie wspiera frameworka arduino” albo brak narzędzi C6.  
**Przyczyna:** oficjalne PlatformIO zostaje przy Arduino 2.x; C6 wymaga Arduino 3.x.  
**Rozwiązanie:** pioarduino, jak w `platformio.ini`.

## Log budowy pokazuje „8MB Flash”

**Objaw:** `HARDWARE: ESP32C6 … 8MB Flash`.  
**Przyczyna:** tekst z JSON-a `esp32-c6-devkitc-1` (referencyjny DevKit).  
**Fakt:** fizyczny flash GEEK to 16 MB; projekt używa `default_16MB.csv` (slot aplikacji 6 553 600 B). To nie jest powód do kasowania flasha ani zmiany tabeli bez analizy.

## LCD skacze przy odświeżaniu

**Było:** co 500 ms całe `fillScreen` + skan ustawia pełny redraw.  
**Teraz:** pełne czyszczenie tylko przy zmianie ekranu/pozycji; liczby idą live.

## Wireshark: File type is neither pcap nor pcapng

**Objaw:** `magic = 0x00000128` (albo inna mała liczba), nie `0xa1b2c3d4`.  
**Przyczyna:** czytelnik FIFO wszedł w środku sesji i dostał `ts_sec` rekordu zamiast nagłówka globalnego (stary mostek, drugi `dd`/Wireshark).  
**Rozwiązanie:** jeden mostek, jeden Wireshark. Zrestartuj `geek_sniffer.py` — każdy nowy reader dostaje `d4 c3 b2 a1`. Sprawdź: `python3 tools/geek_sniffer.py --self-test`.

## Wireshark nie widzi 802.11

Mostek musi iść na FIFO, nie na surowy `/dev/ttyACM0`. LCD: `USB LIVE`. Port nie może być zajęty przez monitor. Szczegóły: [WIRESHARK.md](WIRESHARK.md).

## Nic nie widać na telefonie przy Probe

Normalne. Probe to my → telefon; iOS/Android zwykle milczą. Żeby zobaczyć nazwę: nRF Connect / LightBlue i szukaj `GEEK RADAR`, potem Connect. Na LCD: `PHONE CONNECTED`.

## Probe BLE: TIMEOUT / REFUSED

Long w szczegółach to **jedna** próba. Telefon może odrzucić, wymagać parowania (odrzucone) albo nie być connectable. To nie jest flood. Kolejny Long = kolejna pojedyncza próba.

## Lista BLE pokazuje same adresy

Wiele beaconów nie reklamuje nazwy. Firmware bierze Complete/Shortened Local Name z active scan, potem typ (Watch/Phone/…) i vendor (Apple/Samsung/…) z pól reklamy. Pełny MAC jest w szczegółach. Nie wymyślamy nazw urządzeń.

## LCD czarny / śmieci

**Objaw:** brak splash albo zły obraz.  
**Sprawdź:** piny jak w [HARDWARE.md](HARDWARE.md), `SPI_MODE3`, podświetlenie GPIO6.  
**Nie mieszaj** pinów z nieoficjalnych opisów innych rewizji.

## Przycisk nie zmienia ekranu

**Objaw:** dashboard stoi.  
**Przyczyna:** BOOT to GPIO9 active-low; za krótki impuls odpada na debounce 40 ms.  
**Uwaga:** przytrzymanie BOOT **przy resecie** wchodzi w download mode.

## Long nie otwiera listy

**Było:** Long działał tylko przy puszczeniu między 700 a 1100 ms. Dłuższe trzymanie uzbrajało Repeat i kasowało Long.  
**Teraz:** lista otwiera się w 700 ms trzymania na ekranie WIFI/BLE. Można trzymać dalej.

## Long otwiera listę zamiast skanować

To zamierzone. Na WIFI/BLE dashboard Long = wejście w listę. Rescan: Long na SYSTEM albo Long na pustej liście.

## Przytrzymanie na liście przewija, potem wraca

Na liście Long w 700 ms jest tylko zapamiętany. Repeat od 1100 ms przewija. VeryLong w 1800 ms wraca. Szczegóły: przytrzymaj ~700 ms i puść zanim zacznie się przewijanie.

## Skan Wi‑Fi albo BLE wisi na SCAN

**Objaw:** nagłówek `SCAN` nie znika.  
**Przyczyna:** kolejka — drugi skan czeka aż pierwszy skończy; timeout Wi‑Fi 8 s, BLE ~3.5 s.  
**Rescan:** Long na SYSTEM albo Long na pustej liście.

## Lista BLE „skacze” na inne urządzenie

Nie powinno. Jeśli tak, wybór nie idzie po adresie albo katalog jest sortowany podczas browse. Snapshot i `selectedKey` są w `Navigation` / `AppState`.

## Brak tekstu na monitorze USB

**Objaw:** upload OK, monitor pusty.  
**Prawdopodobna przyczyna:** native USB wymaga CDC (`ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1`). Bez tego `Serial` nie idzie na USB-Serial/JTAG.  
**Uwaga:** po restarcie port USB może na chwilę zniknąć — to zachowanie native USB, nie UART-bridge.
