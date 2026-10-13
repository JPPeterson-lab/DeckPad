# DeckPad als ESP-IDF-Projekt (Versuch)

Gleicher Quellcode wie der Arduino-IDE-Build (`../../src`, `../../Deckpad.ino`), aber mit eigener `sdkconfig.defaults`:
Display-Treiber flash-sicher (`LCD_RGB_ISR_IRAM_SAFE`, `GDMA_ISR_IRAM_SAFE`), PSRAM 120 MHz, Code im PSRAM,
64-KB-Daten-Cache. Zweck: pruefen, ob die Bildstreifen beim Speichern (Flash-Schreibzugriff) verschwinden, und
die Grundlage fuer das ESP32-P4-Board.

**Stand:** baut und laeuft auf dem Board. Arduino-ESP32 3.3, LVGL 8.4 und ArduinoJson kommen als Komponenten. Die
USB-Schicht (HID-Tastatur + Consumer + CDC) ist neu auf `esp_tinyusb` aufgesetzt (`src/usb/usb_device_idf.cpp`), die
Arduino-USB-Klassen werden hier nicht uebersetzt. Die Bildstreifen beim Speichern treten mit dieser Konfiguration nicht auf;
Pixeltakt 30 MHz statt 15 MHz (weniger Flimmern). Ohne USB (`-DDECK_NO_USB=ON`) laeuft der Befehlskanal ueber den UART-Port.

```bash
bash build.sh                       # bauen
bash build.sh flash /dev/cu.xxx     # flashen (UART-Port)
bash build.sh clean                 # nach Aenderungen an sdkconfig.defaults
```
Voraussetzung: ESP-IDF 5.5.x unter `~/esp/esp-idf-v5.5.5` (`./install.sh esp32s3,esp32p4`). Zurueck zum Arduino-Build:
einfach wieder mit der Arduino IDE / `arduino-cli` flashen, die Partitionstabelle ist dieselbe.
