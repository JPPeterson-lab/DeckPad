# DeckPad als ESP-IDF-Projekt (Versuch)

Gleicher Quellcode wie der Arduino-IDE-Build (`../../src`, `../../Deckpad.ino`), aber mit eigener `sdkconfig.defaults`:
Display-Treiber flash-sicher (`LCD_RGB_ISR_IRAM_SAFE`, `GDMA_ISR_IRAM_SAFE`), PSRAM 120 MHz, Code im PSRAM,
64-KB-Daten-Cache. Zweck: pruefen, ob die Bildstreifen beim Speichern (Flash-Schreibzugriff) verschwinden, und
die Grundlage fuer das ESP32-P4-Board.

**Stand (Versuch):** baut. Arduino-ESP32 3.3, LVGL 8.4 und ArduinoJson kommen als Komponenten. **Ohne USB-Geraet**
(`DECK_NO_USB`): Der Befehlskanal laeuft ueber den UART-Port, Tasten werden nicht gesendet. Im Editor
"Alle Ports anzeigen" waehlen und den UART-Port (CH343) verbinden: Profile speichern geht wie gewohnt.

```bash
bash build.sh                       # bauen
bash build.sh flash /dev/cu.xxx     # flashen (UART-Port)
bash build.sh clean                 # nach Aenderungen an sdkconfig.defaults
```
Voraussetzung: ESP-IDF 5.5.x unter `~/esp/esp-idf-v5.5.5` (`./install.sh esp32s3,esp32p4`). Zurueck zum Arduino-Build:
einfach wieder mit der Arduino IDE / `arduino-cli` flashen, die Partitionstabelle ist dieselbe.
