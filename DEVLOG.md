# Entwicklungs-Log

## 2026-10-13 – v0.1.0: Grundgerüst

**Ausgangslage.** Als Vorlage dient ausschließlich der WetterCubePlus XL (gleiches Board, Waveshare ESP32-S3-Touch-LCD-7B). Übernommen wurden `src/hardware/` (RGB-Panel, GT911, I²C, IO-Expander), `src/display/lvgl_port.*` (Teilrendering: LVGL zeichnet 20-Zeilen-Streifen im SRAM und kopiert sie in den PSRAM-Framebuffer) und `build_opt.h`. Gestrichen wurde `-DLODEPNG_NO_COMPILE_ALLOCATORS`: Es verlangt eigene Allokatoren für den PNG-Dekoder (im XL für die Warnkarte), das DeckPad hat keinen und der Linker meldete `lodepng_malloc` als undefiniert.

**USB-Pins.** Die Pins GPIO19/20 teilen sich USB und der CAN-Transceiver; IO5 des IO-Expanders wählt (Header: 0 = USB, 1 = CAN). Das RGB-Panel nutzt GPIO19/20 nicht (Datenleitungen 0, 1, 2, 10, 14, 17, 18, 21, 38–42, 45, 47, 48, dazu 3, 5, 7, 46 für Sync/Takt). Ein Haken steckt im Expander-Treiber: `IO_EXTENSION_Output()` merkt sich den Zustand aller acht Ausgänge und schreibt ihn komplett (`Last_io_value`, am Anfang `0xFF`). Touch-Reset (IO1) und Backlight (IO2) setzen IO5 damit auf 1 = CAN. Das DeckPad setzt IO5 in `usbDeviceInit()` nach der Display-/Touch-Init auf `DECK_USB_SELECT_LEVEL`; weil der Treiber den Zustand mitführt, bleibt es danach dabei. Die Waveshare-Beispieldatei `04_CAN` widerspricht sich an einer Stelle (Kommentar „low“ für CAN, Code und Header „1“ für CAN); der Wert ist deshalb ein `#define`, am Gerät zu bestätigen.

**Arduino-ESP32 3.3.12, USB.** Nötig sind *USB Mode: USB-OTG (TinyUSB)* (`ARDUINO_USB_MODE=0`) und *USB CDC On Boot: Disabled*. Dann ist `Serial` der UART0 (GPIO43/44 → CH343 → USB-C-UART-Port) und der native Port gehört `USB`, `USBHIDKeyboard`, `USBHIDConsumerControl` und `USBCDC` (als `DeckSerial` angelegt, damit niemand es mit `Serial` verwechselt). Tastenkombinationen laufen über `pressRaw()` mit HID-Codes (Modifier 0xE0–0xE7), nicht über die ASCII-Zuordnung des Layouts: Strg+Shift+M ist dann die physische M-Taste, egal welches Layout Windows hat. Nur `text` nutzt das deutsche Layout.

**Schichten.** `hardware/` und `usb/` kennen keine Kacheln. `deck_ui.cpp` kennt LVGL und Profile, aber kein USB: eine Kachel ruft `deckActionRun()` auf, die Aktion geht in eine Queue und wird von einem Task auf Core 0 ausgeführt (der Tastendruck hält 25 ms, das darf LVGL nicht blockieren). LVGL läuft auf Core 1.

**Kacheln.** PicoPixel lieferte 15 statische Container (5 × 3). Sie wurden durch einen Platzhalter `ContainerGrid` ersetzt; die Kacheln entstehen im Code (`buildGrid()`), mit zwei gemeinsamen Styles statt Einzel-Styles pro Objekt. Das Aufleuchten nutzt LVGL-Übergänge: im Zustand *gedrückt* Dauer 0 (sofort), im Normalzustand 350 ms, dadurch blendet die Farbe nach dem Loslassen zurück. Aktionen starten bei `LV_EVENT_PRESSED`, nicht beim Loslassen.

**Profile.** Eine JSON-Datei je Profil auf der LittleFS-Partition `storage` (ArduinoJson 7). Geschrieben wird erst in `<id>.json.tmp`, dann umbenannt, damit ein Stromausfall kein halbes Profil hinterlässt. Fehlerhafte Aktionen (`deckActionValid`) werden beim Laden zu `none`. Ist LittleFS nicht verfügbar, läuft ein Notprofil im RAM, damit das Pad trotzdem Lautstärke steuern kann. IDs sind auf `a-z 0-9 - _` beschränkt (kein `../`). Theme und Helligkeit gehören zum Profil; Helligkeit wird beim Loslassen des Reglers gespeichert, nicht bei jedem Wert.

**Timer.** Zustand liegt außerhalb des Screens (`tmState`, `swRun` …), ein `lv_timer` mit 100 ms läuft immer. Bei Ablauf entsteht ein Vollbild-Objekt auf `lv_layer_top()`: es liegt über jedem Screen und fängt den Touch ab. Blinken: Hintergrund rot/weiß alle 450 ms, dazu die Helligkeit 100 %/20 %; Ende durch Tippen oder nach einer Minute.

**Partitionen.** Ohne OTA entfallen `otadata` und der zweite App-Slot: `nvs` 24 KB, `factory` 4 MB (gleich viel Luft wie ein XL-Slot, genutzt ca. 1 MB), `coredump` 64 KB, `storage` 0xBE0000 = 11,9 MB bis zum Flash-Ende. Die Tabelle wurde gebaut und mit `gen_esp32part.py` zurückgelesen.

**Offen.** Test am Gerät (USB-Polarität, Verbindungs-Ereignis, Touch-Reaktion, Speicher), Profil-Übertragung und Uhrzeit per CDC, Auto-Wechsel nach aktivem Programm, Icons auf den Kacheln.
