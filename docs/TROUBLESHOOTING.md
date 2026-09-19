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

## Brak tekstu na monitorze USB

**Objaw:** upload OK, monitor pusty.  
**Prawdopodobna przyczyna:** native USB wymaga CDC (`ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1`). Bez tego `Serial` nie idzie na USB-Serial/JTAG.  
**Uwaga:** po restarcie port USB może na chwilę zniknąć — to zachowanie native USB, nie UART-bridge.
