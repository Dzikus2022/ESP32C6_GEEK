# Architektura

Dokument opisuje **aktualne** drzewo źródeł. Nie ma jeszcze warstw `core/`, `drivers/`, `services/` ani `network/` — powstaną, gdy pojawi się rzeczywista odpowiedzialność do wydzielenia.

## Stan obecny

Firmware to test USB Serial na ESP32-C6-GEEK. Jedyny kod aplikacji:

```
src/
  main.cpp
```

`src/main.cpp` jest punktem wejścia Arduino (`setup` / `loop`). Na tym etapie cała logika testu mieści się tam (poniżej limitu ~150 linii).

Nie ma jeszcze:

- menedżera Wi‑Fi,
- MQTT / HTTP,
- sterowników GPIO / LCD / TF,
- wspólnego modelu stanu,
- warstwy konfiguracji runtime.

## Przepływ

1. **Boot** — Arduino wywołuje `setup()`.
2. **`setup()`** — `Serial.begin(115200)`, stałe opóźnienie startowe ~2 s (akceptowalne tylko przy starcie), wypisanie potwierdzenia i informacji o chipie (`ESP.getChipModel()`, rewizja, częstotliwość CPU, rozmiar flash, wersja SDK).
3. **`loop()`** — co ~1 s wypisuje `ESP32-C6-GEEK alive`.

Stan aplikacji: brak. Nie ma zmiennych globalnych reprezentujących sprzęt.

Serial idzie przez native USB CDC (flagi w `platformio.ini`). Szczegóły: [CONFIGURATION.md](CONFIGURATION.md), [HARDWARE.md](HARDWARE.md).

## Zależności między modułami

Brak. Jedyny include aplikacji to `<Arduino.h>`.

## Kierunek (niezaimplementowane)

Gdy pojawią się kolejne funkcje, logika biznesowa i sprzętowa ma wyjść z `main.cpp` do osobnych modułów. `main.cpp` zostaje orchestracją. Zasady: `.cursor/rules/00-project-core.mdc`.
