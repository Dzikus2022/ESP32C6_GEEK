# Sprzęt

GPIO tylko z oficjalnych przykładów Waveshare (V1 **i** V2 — LCD taki sam).

Źródła:

- [DEV_Config.h / DEV_Config.cpp](https://github.com/waveshareteam/ESP32-C6-GEEK) — `ESP32-C6-GEEK-Demo` i `ESP32-C6-GEEK_V2-Demo`, przykład `05_LCD_Button`
- [Dokumentacja Arduino](https://docs.waveshare.com/ESP32-C6-GEEK/Arduino) — `PIN_INPUT 9`

## Płytka

| Element | Wartość |
|---------|---------|
| Płytka | Waveshare ESP32-C6-GEEK |
| Chip | ESP32-C6 QFN40, rev 0.2 |
| Flash | 16 MB |
| USB | native USB-Serial/JTAG, `/dev/ttyACM0` |
| LCD | ST7789, 240×135, SPI 4-wire |
| Przycisk | BOOT, active-low |
| PIO board | `esp32-c6-devkitc-1` |

Rewizja **płytki** (V1 vs V2) nie jest potwierdzona w repo. LCD/BOOT poniżej są identyczne w obu demach Waveshare.

## LCD (potwierdzone, V1 i V2)

| Sygnał | GPIO | Źródło |
|--------|------|--------|
| SCLK | **1** | `SPI.begin(1, -1, 2, -1)` |
| MOSI | **2** | j.w. |
| DC | **3** | `DEV_DC_PIN` |
| RST | **4** | `DEV_RST_PIN` |
| CS | **5** | `DEV_CS_PIN` |
| BL | **6** | `DEV_BL_PIN` |

Native panel w `LCD_Driver.h`: 135×240. Firmware: `init(135, 240, SPI_MODE3)`, `setRotation(1)` → 240×135. Waveshare używa SPI mode 3.

## BOOT (potwierdzone)

| Sygnał | GPIO | Uwagi |
|--------|------|--------|
| BOOT | **9** | `#define PIN_INPUT 9`, `OneButton(..., true)` = active-low |

Firmware: `INPUT_PULLUP`. Gestami steruje `ButtonManager` (Short / Long / VeryLong / Repeat) — piny i timing w [CONFIGURATION.md](CONFIGURATION.md). Przy resetcie przytrzymany BOOT to tryb programowania (strapping) — to sprzęt, nie bug.

## TF / SD — nie używać w v0.1

V2 (2026) zmienia okablowanie TF. Przykład wiki (jedna z rewizji) pokazuje m.in. CS 23 / MOSI 18 / MISO 20 / SCK 19 — **nie traktować jako uniwersalne**. Brak SD w firmware, żeby nie zepsuć V1 albo V2.

## Zasilanie

Płytka przez native USB. Wiki: LCD 3.3 V / 5 V. Nie zakładamy innych szyn.
