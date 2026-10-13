# Changelog

*[English version](CHANGELOG.en.md)*

## v0.1.1 (2026-10-13)

### Neu
- **Diagnose-Log:** Ringpuffer mit den letzten 64 Ereignissen (Start inkl. Reset-Grund und Display-Einstellungen, USB-Ereignisse, Tastendrücke, Profilwechsel, **Flash-Schreibzugriffe mit Dauer**). Jede Zeile geht zusätzlich auf den UART-Port
- **COM-Befehle `STATUS` und `LOG`:** `STATUS` liefert eine Zeile mit Version, Laufzeit, freiem und minimalem Speicher (SRAM, PSRAM) und den Flush-Zeiten; `LOG` gibt den Ringpuffer aus
- **Flush-Messung:** Anzahl, mittlere und maximale Dauer des Kopierens in den PSRAM-Framebuffer; alle 10 s eine `[Diag]`-Zeile auf dem UART-Port. Zweck: Streifen und zerrissene Zeilen (PSRAM-Bandbreite) mit Messwerten statt Vermutungen eingrenzen

## v0.1.0 (2026-10-13)

Erstes Grundgerüst. Der Code kompiliert, ein Test am Gerät steht noch aus.

### Neu
- **Projekt:** eigenständiges Projekt auf Basis des WetterCubePlus XL (Treiber für RGB-Panel, GT911, I²C, IO-Expander, LVGL-Teilrendering, `build_opt.h`). Der WetterCubePlus XL selbst blieb unverändert
- **Kachelraster:** die Kacheln entstehen im Code im Platzhalter `ContainerGrid` aus dem PicoPixel-Entwurf. Standard 5 × 3, pro Profil bis 6 × 4. Beim Antippen wechselt die Kachel sofort die Farbe und blendet in 350 ms zurück
- **USB zum PC:** nativer USB-Port als Composite-Gerät mit HID-Tastatur, Consumer/Media-Tasten und virtueller COM-Schnittstelle. `Serial` bleibt auf dem USB-C-UART-Port (Flashen und Debug)
- **USB/CAN-Umschaltung:** IO5 des IO-Expanders wird nach der Display-Init explizit auf USB gesetzt (der XL-Treiber hätte ihn auf CAN stehen lassen)
- **CDC:** zeilenbasierter Befehlsleser mit Befehlstabelle; derzeit antwortet nur `PING` mit `PONG`
- **Aktionen:** `media`, `keys` (z. B. `ctrl+shift+m`), `text`, `host`; die Ausführung läuft in einem eigenen Task, damit LVGL nie auf USB wartet
- **Profile:** je Programm eine JSON-Datei auf der LittleFS-Partition mit eigenen Kacheln und eigenen Einstellungen (Theme, Helligkeit, Raster). Beispielprofile „Standard“ und „Bambu Studio“ beim ersten Start. Auswahl über den Roller in den Einstellungen und die Pfeil-Tasten auf dem Deck
- **Einstellungen pro Profil:** Theme und Helligkeit werden beim Verstellen im aktiven Profil gespeichert (Helligkeit beim Loslassen, um den Flash zu schonen)
- **Taschenrechner** aus dem WetterCubePlus XL, geöffnet über das Rechner-Symbol in den Einstellungen
- **Küchentimer und Stoppuhr:** Timer mit Zeittasten läuft im Hintergrund weiter; bei Ablauf blinkt das Display (Bildschirm und Hintergrundbeleuchtung) auf jedem Screen, bis man es antippt (spätestens nach einer Minute). Stoppuhr mit Rundenzeiten
- **Partitionstabelle ohne OTA:** 4 MB Factory-App, 24 KB NVS, 64 KB Coredump, 11,9 MB LittleFS
- **Werkzeuge:** `tools/deploy_ui.sh` (PicoPixel-Export aufbereiten und prüfen), `tools/bump_version.sh`
- **Doku:** README, Changelog und DEVLOG zweisprachig, Wiki

### Geändert gegenüber dem PicoPixel-Entwurf
- Die 15 statischen Kachel-Container auf dem Deck-Screen wurden durch **einen** Platzhalter-Container `ContainerGrid` (27, 87, 950 × 384) ersetzt. In PicoPixel müssen die alten Container 1–6 und 9–17 noch gelöscht werden; `deploy_ui.sh` meldet sie, solange sie existieren

### Bekannt
- Polarität von IO5 (`0` = USB) und das Verbindungs-Ereignis am Gerät noch nicht bestätigt
- Uhrzeit zeigt `--:--`, bis ein späterer Schritt sie vom PC setzt
