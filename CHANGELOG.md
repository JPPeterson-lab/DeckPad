# Changelog

*[English version](CHANGELOG.en.md)*

## v0.3.2 (2026-10-13)

### Neu
- **Display neu zeichnen:** Knopf im Editor und COM-Befehl `REDRAW`. Zeichnet das ganze Display neu. Ist ein Bildstreifen danach weg, war nur der Bildinhalt gestört; bleibt er, liegt es an der Display-Ansteuerung selbst. Dient der Eingrenzung der Streifen am linken Rand
- **Automatisches Neuzeichnen nach Flash-Schreibzugriffen** (Profil speichern, Icon hochladen, Auswahl sichern): Der RGB-Treiber ist im vorkompilierten Core nicht IRAM-sicher (`CONFIG_LCD_RGB_ISR_IRAM_SAFE` nicht gesetzt), die Ansteuerung setzt beim Schreiben kurz aus. Danach wird einmal der ganze Bildschirm neu gezeichnet; im Log als `[UI] Vollbild neu gezeichnet`

## v0.3.1 (2026-10-13)

### Neu
- **„Tasten am PC ausführen“:** zweiter Schalter im Einstellungs-Screen (Standard: aus). Dann tritt das Board nicht als USB-Tastatur auf, sondern schickt jede Aktion als `DO <typ> <angabe>` über den COM-Port, und der **Windows-Helfer** führt Tasten, Medientasten, Text und Programmstarts aus (`SendInput`, `Start-Process`). Gedacht für Boards ohne USB-Tastatur (z. B. das geplante ESP32-P4-Board) und als Ausweichweg. Ohne Helfer passiert nichts (Hinweis im Log und am Display)
- Der Helfer hört jetzt alle 40 ms auf das Gerät (vorher nur 0,6 s), das Vordergrundfenster wird weiter alle 0,6 s geprüft

### Behoben
- **Tastenkürzel mit Z oder Y waren auf deutschen Tastaturen vertauscht:** Buchstaben und Ziffern wurden als *physische* Taste der US-Belegung gesendet, auf deutscher Belegung ist das Z dort das Y (`ctrl+z` kam als Strg+Y an). Jetzt folgen Buchstaben und Ziffern dem Layout des PCs (`DECK_KEY_LAYOUT`, Standard Deutsch). Betrifft z. B. das Beispielprofil „Bambu Studio“ (Rückgängig). Strg+Shift+M, Strg+C/V, Strg+S waren nicht betroffen. Wer einen PC mit US-Belegung nutzt, stellt `DECK_KEY_LAYOUT` auf `KeyboardLayout_en_US`

## v0.3.0 (2026-10-13)

### Neu
- **Icons auf den Kacheln:** Symbole (Emoji-Auswahl) oder eigene Bilder (PNG, JPG, SVG …), im Editor in 32–80 px umgewandelt und per COM-Port ins Gerät geladen (RGB565 + Alpha, `/icons/<id>.bin`, bis 96 × 96 px). Mehrere Profile können dasselbe Icon nutzen. Die Schnellauswahl im Editor bringt passende Symbole mit. COM-Befehle `ICONBEGIN`, `ICONDATA`, `ICONEND`, `ICONLIST`, `ICONGET`, `ICONDEL`
- **Automatischer Profilwechsel (einstellbar):** Schalter „Automatischer Profilwechsel“ im Einstellungs-Screen (Standard: aus). Ein kleiner **Windows-Helfer** (`tools/helper/deckpad-helper.ps1`, ohne Installation) meldet das Programm im Vordergrund; das DeckPad wählt das Profil mit passendem Feld „Programm“ (z. B. `bambu-studio.exe`), sonst „Standard“. Der Einstellungs-Screen zeigt, ob der Helfer erkannt wird. COM-Befehl `ACTIVE <programm.exe>`
- **Editor:** Knopf **„Trennen“** gibt den COM-Port frei (der Helfer gibt ihn außerdem von selbst frei, solange der Editor im Vordergrund ist)
- **Statusanzeige:** „PC verbunden“ ist grün hinterlegt, „Kein PC“ rot

## v0.2.1 (2026-10-13)

### Geändert
- **Profilauswahl wird verzögert gespeichert:** bisher schrieb jeder Profilwechsel sofort in den Flash. Flash-Schreibzugriffe stoppen kurz den Cache (und damit den PSRAM-Zugriff des Displays) und verschleißen den Flash. Jetzt wird die Auswahl erst geschrieben, wenn 8 s lang nicht gewechselt wurde (`[Flash]`-Zeile im Log). Mögliche Ursache der Streifen nach mehreren Profilwechseln, noch nicht bestätigt

### Neu
- **Mehr Messwerte im Log:** Dauer des Kachel-Aufbaus bei jedem Profilwechsel, und einzelne langsame Flushes (länger als 20 ms) mit Größe und Position des Bereichs

## v0.2.0 (2026-10-13)

### Neu
- **DeckPad Editor** (`tools/editor/deckpad-editor.html`): Profile am **Windows-PC** im Browser (Chrome/Edge, Web Serial) anlegen, bearbeiten, kopieren, löschen und aktivieren. Kacheln anklicken, Beschriftung, **Farbe** (Farbwähler und Palette), **Aktion** (Medien, Tastenkombination mit Modifier-Kästchen und Tastenaufnahme, Programm starten, Text, Host-Nachricht) und Schnellauswahl-Vorlagen. Profil-Sicherung als Datei, Demo-Modus ohne Gerät. Setzt beim Verbinden die **Uhrzeit** des PCs. Zeigt Status und Geräte-Log
- **Editor online:** über GitHub Pages erreichbar (<https://jppeterson-lab.github.io/DeckPad/>), nichts zu installieren. `tools/deploy_editor.sh` kopiert die Datei nach `docs/index.html`
- **Editor erkennt den falschen Port:** der Auswahldialog zeigt nur Espressif-Geräte (VID 0x303A), also den nativen USB-Port; der UART-Port (CH343) wird erkannt und erklärt. „Alle Ports anzeigen“ hebt den Filter auf
- **Aktion `run`:** startet ein Programm, eine Datei oder einen Link über Win+R (Programmname, Pfad, Internetadresse). Das Gerät drückt Win+R, tippt den Text und bestätigt mit Enter
- **COM-Befehle:** `VER`, `LIST`, `GET <id>`, `PUT <id> <json>`, `DEL <id>`, `USE <id>`, `TIME <Unix-Sekunden>` (zusätzlich zu `PING`, `STATUS`, `LOG`). Das Gerät übernimmt Änderungen sofort (Roller, Raster, Farben)
- **Schichten:** die USB-Schicht kennt nur den Kanal; Befehle werden von `app/host_commands.cpp` angemeldet

### Geändert
- Zeilenlänge der COM-Befehle von 256 auf 6144 Byte, Stack des CDC-Tasks auf 8 KB (ein Profil als JSON braucht bis ca. 4 KB)

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
- Das Verbindungs-Ereignis am Gerät ist noch nicht bestätigt (die Polarität von IO5, `0` = USB, bestätigt die Waveshare-Dokumentation)
- Uhrzeit zeigt `--:--`, bis ein späterer Schritt sie vom PC setzt
