# Credits & Drittanbieter-Lizenzen

Dieses Projekt verwendet Ressourcen Dritter. Die jeweiligen Lizenzbedingungen
sind nachfolgend aufgeführt.

---

## Treiber

**Quelle:** Waveshare-Beispielcode für das ESP32-S3-Touch-LCD-7B (RGB-Panel, I²C, IO-Expander) sowie der GT911-Treiber auf Basis von Espressif `esp_lcd_touch`
(Apache License 2.0, © Espressif Systems (Shanghai) CO LTD). Siehe Dateikopf in `src/hardware/`.

---

## Bibliotheken

| Bibliothek | Lizenz |
|---|---|
| [LVGL](https://lvgl.io) 8.3 (inkl. lodepng) | MIT / zlib |
| [ArduinoJson](https://arduinojson.org) 7 | MIT |
| Arduino-ESP32 / ESP-IDF | LGPL 2.1 / Apache 2.0 |

## Icons der Oberfläche

**Quelle:** [Flat Color Icons](https://github.com/icons8/flat-color-icons) von [Icons8](https://icons8.com/color-icons)

**Lizenz:** MIT oder [Good Boy License](https://icons8.com/good-boy-license/) (frei für private und kommerzielle Nutzung, Namensnennung nicht erforderlich, hier trotzdem genannt)

Verwendet werden die Symbole Haus, Zahnrad, Taschenrechner, Wecker und die beiden Pfeile. Sie sind als Bilddaten in die Firmware eingebettet (`src/ui/images/`).

---

## Tastaturlayouts

`src/usb/keyboard_layouts.h`: ASCII→HID-Tabellen (Deutsch, US) aus der USB-Bibliothek von Arduino-ESP32 (`USBHIDKeyboard`, LGPL-2.1).

## Oberfläche & Schrift

- UI-Design und Code-Export: [PicoPixel](https://picopixel.io)
- Schrift: Montserrat (SIL Open Font License 1.1)

---

## Vorlage

Treiber, LVGL-Anbindung, Build-Einstellungen und der Taschenrechner stammen aus dem [WetterCubePlus XL](https://github.com/JPPeterson-lab/WetterCubePlus-XL) (gleiches Board).
