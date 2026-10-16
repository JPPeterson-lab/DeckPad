# Entwicklungs-Log

## 2026-10-16 – v0.6.1: fünf Schnelltasten, Bootstrap Icons

Die UI wurde erneut überarbeitet: `buttonp4` und `buttonp5` sind dazugekommen, alle Symbole sind jetzt Bootstrap Icons (`bs_*`-Bilder). Die Schnellwahl hängt nur an der Konstante `DECK_QUICK_COUNT` (3 → 5), dem Objekt-Lookup in `quickBtn()` und dem Editor; in `config.json` ist `quick` ein Array, ein altes mit drei Einträgen liest sich unverändert (fehlende Einträge = frei). `QUICK` verlangt jetzt genau fünf Angaben, ein alter Editor mit drei bekommt `ERR`. Die Icons8-Symbole sind nirgends mehr im Einsatz, deshalb ersetzen die Bootstrap Icons sie in den Credits (MIT verlangt den Copyright-Vermerk, steht in `CREDITS.md`). `bump_version.sh` passt jetzt auch die „Beta (vX)“-Zeile im README an.

## 2026-10-16 – v0.6.0: Schnelltasten, Beta

**Schnelltasten.** Die neue UI bringt vier Tasten unter dem Raster (`buttonstandard`, `buttonp1..3`). Sie liegen auf dem Deck-Screen und nicht im Kachelfeld, das beim Profilwechsel neu gebaut wird; damit sind sie auf jeder Profilseite da, ohne dass der Code sie je neu anlegen muss (alle Profile teilen sich diesen einen Screen). „Standard“ ist fest an die Profil-ID `standard` gebunden. P1–P3 stehen als ID-Liste `quick` in der `config.json` (geräteweit wie Design und Helligkeit, verzögert geschrieben). Eine ID, die es nicht mehr gibt, zählt als frei, die Datei wird dafür nicht angefasst. Der Editor setzt die Belegung mit `QUICK`, `LIST` liefert sie zurück. Optik ohne neue Grafik: frei = 30 % Deckkraft und nicht klickbar, aktives Profil = 3 px blauer Rand.

**Beta.** Entscheidung des Nutzers nach dem intensiven Test von 0.5.0. Freie Plätze und geräteweites Design stehen in der Statustabelle jetzt auf „geprüft“; ungetestet bleiben Rechner, Timer, Windows-Helfer und Helfer-Modus, die Schnelltasten sind neu. Bis 1.0 können sich Formate und Befehle noch ändern.

## 2026-10-15 – Windows-Helfer als richtiges Programm

Das PowerShell-Skript wirkte wie ein Provisorium (Konsolenfenster, Ausführungsrichtlinie, `.cmd`-Starter). Der Ersatz ist ein kleines C#-Programm (WinForms, .NET Framework 4.8): das ist in Windows 10/11 schon vorhanden, die exe bleibt dadurch bei ca. 80 KB und braucht keine Installation. Gebaut wird auf dem Mac mit dem .NET-8-SDK (`~/.dotnet`) und den Referenzassemblies von NuGet (`tools/build_helper.sh`). **Nur gebaut, auf dem Mac nicht lauffähig:** die Logik (WMI-Portsuche nach 303A:4DC0, `ACTIVE`-Meldung, `DO`-Aktionen per `SendInput`, Port freigeben, solange der Editor im Vordergrund ist) ist 1:1 aus dem Skript übernommen und muss unter Windows getestet werden. Entscheidung des Nutzers: der Helfer soll nicht dauerhaft im Autostart laufen, deshalb sind *Mit Windows starten* (Registry `HKCU\…\Run`, `--tray`) und *Beenden, wenn das DeckPad getrennt wird* beide standardmäßig aus. Die exe ist nicht signiert (Zertifikat kostet), Windows zeigt SmartScreen.

## 2026-10-15 – v0.5.0: freie Plätze, geräteweites Design, nur noch ESP-IDF

**Design pro Profil war mein Entwurf, nicht gewollt.** Der Nutzer meldete: Nach dem Umschalten auf dunkel sind Hauptprofil und Einstellungen dunkel, aber andere (neu angelegte) Profile nicht. Ursache: `theme` und `brightness` standen im Profil (Entwurfsentscheidung aus „Einstellungen pro Programm“), der Schalter schrieb das Design nur in das aktive Profil, und der Editor legte neue Profile mit `light` an. Auf Rückfrage soll auch die Helligkeit nicht pro Profil gelten. Beides ist jetzt geräteweit (`config.json`: `theme`, `brightness`), verzögert gespeichert wie die Profilauswahl; das schont zugleich den Flash (vorher schrieb jedes Loslassen des Helligkeitsreglers die Profildatei). Migration: fehlen die Schlüssel in `config.json`, werden Design und Helligkeit des aktiven Profils übernommen, damit sich beim Update nichts ändert. Die Felder bleiben in `DeckProfile` nur zum Lesen alter Dateien.

**Freie Plätze.** Das Kachelfeld war eine dichte Liste (Platz = Index), Lücken ließen sich nur mit leeren „Füll“-Kacheln darstellen, die dann als farbige Kachel erschienen. Jetzt hat jede Kachel einen `slot`; `fromJson` ignoriert Plätze außerhalb des Rasters und doppelte, `buildGrid` ordnet über eine Tabelle Platz → Kachel zu (der Kachelindex bleibt Schlüssel für Icons und Ereignisse). Im Editor Ziehen und Ablegen mit der HTML5-API (verschieben oder tauschen). Rasteränderung: Kacheln behalten (Zeile, Spalte).

**Nur noch ESP-IDF.** Entscheidung des Nutzers nach dem Test: S3-Board bleibt, komplett auf ESP-IDF. Das Projekt wanderte (`git mv`, Verlauf bleibt) in den Wurzelordner; entfernt wurden `Deckpad.ino` (Inhalt jetzt `main/main.cpp`), `build_opt.h`, `platforms/`, die Arduino-USB-Implementierung und der `DECK_NO_USB`-Sonderweg. Arduino-ESP32 bleibt als Komponente, weil `Serial`, `String`, LittleFS und ArduinoJson im Code stecken; sie ganz herauszulösen wäre ein eigener Schritt. Das P4-Board kommt in ein WetterCube-Projekt.

---

## 2026-10-13 – Web-Installer

**Aufbau.** Wie im WetterCubePlus XL: `esp-web-tools` (`<esp-web-install-button>`) auf GitHub Pages, Manifest und Firmware unter `docs/`. Anders als im XL (vier Teile, Arduino-Export) gibt es hier **ein zusammengeführtes Image** (`esptool merge_bin`, `deckpad-merged.bin` ab Offset 0, 1,1 MB): Bootloader, Partitionstabelle und Programm. `tools/prepare_release.sh` baut mit ESP-IDF, führt zusammen, prüft die Teile im Image und die Versionsnummer im Programm und schreibt `manifest.json`/`version.json`. Die Datenpartition `storage` (Profile, Icons) liegt hinter dem Image und bleibt beim Update erhalten; „Gerät löschen“ (`new_install_prompt_erase`) löscht sie.

**Stolpersteine, die auf der Seite stehen.** Installiert wird über den **UART-Port** (CH343, setzt den Chip per DTR/RTS in den Bootloader). Der native USB-Port trägt im Normalbetrieb die DeckPad-Firmware und kann den Chip nicht in den Bootloader schalten; außerdem bricht das Schreiben ab, wenn er gleichzeitig am PC hängt (siehe oben). Der Editor wurde auf `editor.html` verschoben, damit die Startseite der Installer ist.

**Entscheidung.** Mittelfristig wird von Arduino IDE auf ESP-IDF gewechselt; wer nicht den Installer nutzt, soll sich im ESP32-Umfeld auskennen (README/Seite sagen das). Die Installation selbst über den Browser ist noch nicht ausprobiert.

---

## 2026-10-13 – USB unter Windows: PID-Kollision

**Befund.** IDF-Build am Windows-PC: Das Display zeigt „PC verbunden“, aber Tasten tun nichts und der Editor kann den Port nicht öffnen („Failed to open serial port“); der Port heißt jetzt COM9 statt COM6 (neue Seriennummer, erwartet). `Get-PnpDevice`: Das Gerät und der COM-Port sind „OK“, die Schnittstelle MI_02 („DeckPad Tasten“) hat aber die Klasse **USBDevice** statt HIDClass. Am Mac dagegen alles in Ordnung: HID-Tastatur und Consumer mit korrekter Berichtsbeschreibung (Usage-Paare 1/6 und 12/1), CDC antwortet auf `PING`/`VER`/`STATUS`. Die UART-Aufzeichnung zeigte, dass die Tastendrücke das Gerät erreichen und gesendet werden.

**Ursache (Verdacht, durch den Mac-Test eingegrenzt).** 303A:1001 ist die Kennung der USB-JTAG/Seriell-Schnittstelle der Espressif-Chips; deren Windows-Treiber (WinUSB, z. B. durch die IDF/OpenOCD-Einrichtung) bindet sich an `USB\VID_303A&PID_1001&MI_02`. In der Arduino-Firmware lag die Tastatur an anderer Stelle (die Reihenfolge der `begin()`-Aufrufe bestimmt die Interface-Nummern), in der IDF-Firmware ist MI_02 die Tastatur: Windows legt dort WinUSB statt HID an.

**Maßnahme.** Eigene Produkt-ID `303A:4DC0` (`DECK_USB_PID`, für beide Builds), Helfer sucht zuerst `PID_4DC0`, dann `PID_1001`. Provisorisch: Für eine Veröffentlichung eine eigene PID beantragen (Espressif vergibt sie kostenlos, alternativ pid.codes mit VID 0x1209). Am Mac verifiziert, am PC offen.

---

## 2026-10-13 – ESP-IDF: Ergebnis, Pixeltakt, eigene USB-Schicht

**Streifen weg.** Auf dem Board mit dem IDF-Build (Display-ISR flash-sicher, PSRAM 120 MHz, Code im PSRAM, 64-KB-Cache): fünf Profil-Speichervorgänge hintereinander, keine Streifen. Die Schreibzeiten sinken auf 29–93 ms (Arduino: 68–199 ms). Damit ist die Ursache bestätigt: die vorkompilierte Core-Konfiguration.

**Flimmern.** 15 MHz Pixeltakt ergeben bei 1386 × 661 Bildpunkten (Timing des Panels) nur 16,4 Bilder/s. Der niedrige Takt war im XL der Kompromiss gegen Streifen (PSRAM-Bandbreite bei 80 MHz). Mit 30 MHz (33 Hz) „flimmert es nicht mehr“. Der Takt ist über `DECK_PIXEL_CLOCK_HZ` einstellbar (IDF: `PCLK=40 bash build.sh`); 40 MHz ≈ 44 Hz, 51 MHz ≈ 56 Hz bleiben als Stufen.

**USB.** Arduino-USB-Klassen lassen sich in der IDF-Komponentenform nicht nutzen: Sie erwarten `CONFIG_TINYUSB_ENABLED` (existiert in `esp_tinyusb` 1.x und 2.x nicht) und eine eigene TinyUSB-Anbindung. Die USB-Schicht war bewusst vom Rest getrennt (`src/usb/`), daher ließ sie sich ersetzen: `usb_device_idf.cpp` baut über die `esp_tinyusb`-API einen Verbund aus CDC + HID (Tastatur Report-ID 1, Consumer Report-ID 2), VID/PID wie bisher (303A:1001, damit Editor und Helfer das Gerät finden), feste Seriennummer (Windows vergibt dieselbe COM-Nummer). Die Arduino-Quellen `USB*.cpp` werden per CMake aus dem Build genommen. Layout: Die ASCII→Tastencode-Tabellen (Deutsch/US) stammen aus der Arduino-USB-Bibliothek (`keyboard_layouts.h`), damit Text und Buchstaben-Kürzel dasselbe tun wie im Arduino-Build. Das Modul ist dieselbe Schnittstelle (`usb_device.h`), `usb_cdc.cpp`, Aktionen und Profile blieben unverändert.

**Offen:** Test am PC (Tastatur, Medientasten, COM-Port), `usbHostConnected()` über Attach/Detach-Ereignisse, Verhalten bei Standby.

---

## 2026-10-13 – ESP-IDF-Versuch (platforms/idf)

**Ausgangslage.** Panel-Neustart (v0.3.3) hilft nicht gegen die Streifen nach dem Speichern. Die Ursache liegt in der vorkompilierten Core-Konfiguration (`LCD_RGB_ISR_IRAM_SAFE` aus, PSRAM 80 MHz, kleiner Cache), die der Sketch nicht ändern kann. Auf dem Mac lag bereits ESP-IDF 5.3.2 samt Projekt `zentralsteuerung` (gleiches Board, erprobte Werte: PSRAM 120 MHz, Code im PSRAM, 64-B-Cache-Zeilen). Daneben wurde **ESP-IDF 5.5.5** installiert (passt zum Core 3.3.12 und ist die Mindestversion fürs ESP32-P4-Board).

**Aufbau.** `platforms/idf/` bindet denselben Quellcode ein (`../../src/**`, `Deckpad.ino` über `main.cpp`), Arduino-ESP32 3.3, LVGL 8.4 und ArduinoJson als Komponenten (Komponentenmanager). `conf/lv_conf.h` ist eine Projektkopie der gemeinsamen `lv_conf.h` mit den Schaltern aus `build_opt.h`. Die `sdkconfig.defaults` setzt `LCD_RGB_ISR_IRAM_SAFE`, `GDMA_ISR_IRAM_SAFE`, PSRAM 120 MHz OPI, `SPIRAM_FETCH_INSTRUCTIONS`/`SPIRAM_RODATA`, 64-KB-Daten-Cache.

**Hürden beim ersten Build** (alle gelöst): (1) LVGL fand `lv_conf.h` nicht: Include-Pfad `conf/`; (2) Fremdkomponente `esp-modbus` (Abhängigkeit der Arduino-Komponente) scheitert in IDF 5.5 an `-Werror=stringop-truncation` und hängt ein eigenes `-Werror` an: gezielte `-Wno-error=…`; (3) die Arduino-Klassen `USBCDC`/`USBHID*` brauchen `CONFIG_TINYUSB_ENABLED`, das es in IDF 5.5 + `esp_tinyusb` 1.x nicht gibt (der Arduino-Core nutzt eine eigene TinyUSB-Anbindung). Für den Versuch gibt es `DECK_NO_USB`: Der Befehlskanal läuft über den UART-Port, Tasten werden nicht gesendet. Für den Dauerbetrieb muss die USB-Schicht neu gebaut werden (`esp_tinyusb`-API: Deskriptoren für HID-Tastatur, Consumer und CDC). Python-Umgebung: Der Installer hatte das venv auf `/Applications/Xcode.app/…/python3` gesetzt, das später nicht mehr existierte; Verweis auf die Command-Line-Tools-Python umgebogen.

**Ergebnis bisher.** Baut (1,0 MB von 4 MB). Wirkung auf die Streifen: **noch nicht getestet.** Der Arduino-Build ist unverändert (gleiche Größe).

---

## 2026-10-13 – v0.3.3: Panel nach Flash-Schreibzugriff neu starten

**Log vom Gerät (v0.3.2).** Streifen traten auf, nachdem im Editor ein Icon und das Profil gespeichert wurden (Icon 103 ms, Profil 90 ms). Das automatische Neuzeichnen lief danach (`[UI] Vollbild neu gezeichnet`, 29,5 s), die Streifen blieben. Ein vollständiges Neuzeichnen schreibt den Framebuffer komplett neu: Der Bildinhalt im PSRAM ist danach korrekt, also kann der Fehler nicht im Framebuffer liegen. Er liegt in der Ansteuerung des Panels (Bounce-Buffer/DMA aus dem Takt).

**Maßnahme.** `esp_lcd_rgb_panel_restart()` (IDF-Doku: „to save the screen from a permanent shift, you can call this function to restart the LCD DMA“). Es setzt ein Flag, der Neustart erfolgt im nächsten VSYNC-Handler. `CONFIG_LCD_RGB_RESTART_IN_VSYNC=y` ist gesetzt und soll das automatisch tun; offenbar reicht das nach einem Cache-Stillstand nicht, deshalb wird es nach jedem Flash-Schreibzugriff ausdrücklich angestoßen. Zum Testen gibt es `PANEL` und einen Editor-Knopf, getrennt von `REDRAW`, damit sich Bildinhalt und Ansteuerung unterscheiden lassen.

**Bewertung.** Das ist weiter eine Hypothese: Wirkt `PANEL`, ist die Ursache bestätigt. Wirkt es nicht, bleibt die nicht IRAM-sichere Konfiguration des vorkompilierten Cores (nur mit eigenem Core oder ESP-IDF änderbar).

---

## 2026-10-13 – v0.3.2: Streifen kommen mit Icons wieder

**Log vom Gerät.** Beim Hochladen und Speichern vieler Icons und Profile tauchen die Streifen wieder auf. Im Log: **kein einziger Eintrag `[Flush] langsam`**, auch nicht während der Flash-Schreibzugriffe (Profil 68–199 ms, Icon 30–63 ms, Auswahl 10–14 ms), also kein Flush über 20 ms. Der Profilwechsel kostet 12–28 ms, mit den neuen Icon-Kacheln einmal 71 ms. Damit ist ausgeschlossen, dass ein einzelner langer Kopiervorgang ins PSRAM die Ursache ist (die ursprüngliche XL-Hypothese). Übrig bleibt die Display-Ansteuerung selbst.

**Konfiguration.** `CONFIG_LCD_RGB_ISR_IRAM_SAFE` und `CONFIG_GDMA_ISR_IRAM_SAFE` sind im vorkompilierten Arduino-Core **nicht** gesetzt (`CONFIG_LCD_RGB_RESTART_IN_VSYNC=y` ist gesetzt). Beim Schreiben in den Flash steht der Cache still; der Interrupt, der den Bounce-Buffer des Panels füllt, liegt im Flash-Cache und kann in dieser Zeit nicht laufen. Das Panel wird dann nicht mehr bedient (Flackern, wie beim OTA-Update im XL). Aus dem Sketch lässt sich das nicht ändern, es bräuchte einen Core mit anderer Konfiguration (Arduino-Lib-Builder oder ESP-IDF).

**Was noch offen ist.** Ob die dauerhaften Streifen aus solchen Aussetzern entstehen oder eine andere Ursache haben, ist nicht belegt. Zwei Hilfen: (1) nach jedem Flash-Schreibzugriff zeichnet das Gerät den ganzen Bildschirm einmal neu (falls der Bildinhalt gestört wurde); (2) `REDRAW`/Editor-Knopf „Display neu zeichnen“ als gezielter Test: Verschwindet der Streifen damit, steckt der Fehler im Bildinhalt (Framebuffer), bleibt er, in der Ansteuerung des Panels. Außerdem zeigt das Log jetzt, ob der Streifen zeitlich zu einer `[Flash]`-Zeile passt.

---

## 2026-10-13 – v0.3.1: Aktionen über den Helfer, Layout-Fehler

**Fehler in meiner Annahme (behoben).** In v0.1.0 hatte ich Tastenkombinationen über HID-Codes der *physischen* Taste gesendet und das als Vorteil dargestellt („unabhängig vom Windows-Layout“). Das stimmt nur für Tasten, die in beiden Layouts an derselben Stelle dasselbe Zeichen liefern. Bei Y/Z ist es falsch: `ctrl+z` kam auf deutscher Belegung als Strg+Y an, das mitgelieferte Beispielprofil „Bambu Studio“ hatte damit „Rückgängig“ und „Wiederholen“ vertauscht. Aufgefallen ist es bei der Planung des Helfer-Modus, weil `SendInput` mit virtuellen Tastencodes (VK) dem Layout folgt und beide Wege sonst unterschiedlich reagiert hätten. Jetzt gehen Buchstaben und Ziffern auch am Board über das Layout (`usbKeyChord` nimmt `0x100 | Zeichen`, die Bibliothek löst es über die Layout-Tabelle auf), Sondertasten und Modifier bleiben physisch. Das Layout ist eine Einstellung im Code (`DECK_KEY_LAYOUT`), weil das Board das Layout des PCs nicht kennen kann.

**Aktionen über den Helfer.** Anlass ist das ESP32-P4-Board: Dessen USB-Anschlüsse erlauben ohne Bastelei keine Tastatur (siehe Schaltplan: UART-Port am CH343, „USB1.1“ am festen Serial/JTAG-Controller). Der Modus `hostactions` leitet jede Aktion als `DO …` über den COM-Port um; der PowerShell-Helfer setzt sie mit `SendInput` (Tasten, Text als Unicode-Zeichen) und `Start-Process` um. `Start-Process` ist für Programmstarts besser als Win+R, weil kein Dialog nötig ist. Medientasten nutzen die VK-Codes (`VK_VOLUME_UP` …). Das Verhalten ist beim S3 identisch zur USB-Tastatur, nur mit einem Programm dazwischen und etwas mehr Verzögerung.

**Ungetestet:** Helfer-Aktionen (PowerShell/`SendInput`), Schalter, Layout-Korrektur am Gerät.

---

## 2026-10-13 – v0.3.0: Icons, Auto-Profilwechsel, Statusfarbe

**Icons.** Format: 4 Byte Kopf (`DI`, Breite, Höhe) und danach 3 Byte je Pixel (RGB565 low/high, Alpha), das ist LVGL `LV_IMG_CF_TRUE_COLOR_ALPHA` bei 16 Bit Farbtiefe, sodass das Gerät die Datei direkt als `lv_img_dsc_t` verwenden kann. Der Browser rechnet um: Emoji oder Bild auf ein Canvas (contain, Transparenz bleibt), `getImageData`, RGB565. Übertragung als Base64 in Zeilen zu 3200 Zeichen (2400 Byte), weil eine COM-Zeile höchstens 6 KB hat. Die Pixeldaten liegen im PSRAM, solange die Kachel existiert, und werden nach `lv_obj_clean` freigegeben. Sie werden beim Aufbau aus dem Flash gelesen (kein Dauer-Cache; 15 Kacheln × 48 × 48 × 3 Byte ≈ 100 KB). Bilder im PSRAM belasten die gleiche Bandbreite wie der Framebuffer, daher klein halten (Hinweis im Editor). Die Kachel nutzt Flex-Column (Icon oben, Text darunter).

**Auto-Profilwechsel.** Ein USB-Gerät kann das Vordergrundfenster des PCs nicht sehen, also braucht es einen Helfer. Ein PowerShell-Skript (`System.IO.Ports.SerialPort`, `GetForegroundWindow`) schickt `ACTIVE <exe>` alle 0,6 s bei Änderung und sonst alle 3 s als Lebenszeichen. Problem: Ein COM-Port gehört genau einem Programm. Ein **zweiter CDC-Port** wäre sauberer (Core 3.3.12 kennt zwei, laut Sdkconfig `CONFIG_TINYUSB_CDC_MAX_PORTS=2`), aber die Endpunkte des ESP32-S3 sind knapp (HID + 2 × CDC ≈ 5 von 6) und ein Fehler hätte die funktionierende USB-Verbindung zerstört, ohne dass ich es am Gerät prüfen kann. Gewählt ist ein einfacherer Weg: Der Helfer gibt den Port frei, solange ein Fenster mit „DeckPad Editor“ im Titel im Vordergrund ist. Der Editor hat außerdem einen Knopf „Trennen“. Nachteil: Während der Editor verbunden bleibt und im Hintergrund liegt, wechselt die Automatik nicht. Wechselt das Gerät zu „Standard“, wenn zum Programm kein Profil existiert, springt es bei jedem Fensterwechsel dorthin; das ist Absicht (wie beim Stream Deck), der Schalter erlaubt, es auszuschalten. Der Schalter ist Teil von `config.json` (`autoswitch`) und wird wie die Auswahl verzögert geschrieben.

**Oberfläche.** Schalter und Hinweis entstehen im Code im Einstellungs-Screen (unter den beiden Containern), nicht in PicoPixel, damit kein neuer Export nötig ist; Farben folgen dem Theme. „PC verbunden“/„Kein PC“ bekommen Hintergrund grün/rot; `change_color_theme` setzt die Textfarbe zurück, daher wird die Statusfarbe nach jedem Theme-Wechsel neu gesetzt.

**Ungetestet am Gerät.** Icon-Übertragung und -Darstellung, Schalter, `ACTIVE` und das PowerShell-Skript (auf dem Mac gibt es keine PowerShell).

---

## 2026-10-13 – v0.2.1: Streifen nach Profilwechseln

**Befund vom Gerät.** USB, Editor, Profile, Tasten und `run` funktionieren (Log vom Gerät: USB-Ereignis „gestartet“ nach 1,3 s, `Profil 'notepad' geschrieben: ok, 116 ms`). Die Streifen am linken Rand sind **nach dem Boot sauber und tauchen nach mehreren Profilwechseln wieder auf**. Der Flush-Zähler war in diesem Log nicht dabei (nur `LOG`, nicht `STATUS`).

**Verdacht, nicht bestätigt.** Jeder Profilwechsel schrieb `config.json` in den Flash (`saveConfig()`), ohne dass es im Log stand. Ein Schreibvorgang dauert laut Log bis über 100 ms und stoppt den Cache; der RGB-Treiber ist nicht IRAM-sicher (`CONFIG_LCD_RGB_ISR_IRAM_SAFE`, siehe XL-DEVLOG „OTA-Test: Display flackert“). Dazu kommt der Neuaufbau von 15 Kacheln mit Theme-Wechsel in einem Zug. Das passt zu „nach ein paar Mal“, ist aber noch keine Messung. Gegenmaßnahme ohne Nachteil: die Auswahl erst 8 s nach dem letzten Wechsel schreiben. Zusätzlich wird jeder Wechsel mit der Aufbaudauer protokolliert und jeder Flush über 20 ms mit Bereich ins Log geschrieben, damit sich Streifen einem Ereignis zuordnen lassen.

**Ergebnis vom Gerät (v0.2.1).** Rund 25 Profilwechsel hintereinander: „sieht besser aus“. Im Log keine einzige Zeile `[Flush] langsam` (also kein Flush über 20 ms), der Kachel-Aufbau dauert 12–27 ms, die Auswahl wurde nur nach Pausen geschrieben (`[Flash] … 16–19 ms`). Das stützt den Verdacht auf die Flash-Schreibzugriffe, beweist ihn aber nicht: Es ist keine Messung vor und nach der Änderung mit gleichem Ablauf, und ob die Streifen ganz verschwunden sind, ist nicht festgehalten. Offen bleiben Schreibzugriffe, die sich nicht verschieben lassen (z. B. Profil vom Editor speichern, ca. 116 ms). Beobachten, ob Streifen kurz nach einer `[Flash]`-Zeile auftauchen.

**Vorgehen.** Wie im XL nur eine Änderung auf einmal: nach v0.2.1 mehrfach Profile wechseln. Tauchen die Streifen weiter auf, `LOG` und `STATUS` direkt danach lesen (`[Flush] langsam`, `[UI] … Aufbau … ms`) und die nächste Maßnahme wählen (Raster-Aufbau in Teilen, Übergänge abschalten, Pixeltakt senken).

---

## 2026-10-13 – v0.2.0: Editor am PC und Programmstart

**Anlass.** Entwickelt wird am Mac, benutzt wird alles am Windows-PC. Profile mussten deshalb von Windows aus erstellbar sein, ohne Werkzeuge zu installieren: eine einzelne HTML-Datei mit **Web Serial** (Chrome/Edge), die die COM-Schnittstelle des DeckPad direkt anspricht.

**Protokoll.** Zeilenbasiert; `PUT <id> <json>` überträgt ein Profil als eine Zeile. Das Gerät prüft (`fromJson` kappt zu lange Texte und ersetzt ungültige Aktionen durch `none`), schreibt über Temp-Datei und Umbenennen und aktualisiert dann die Oberfläche.

**Threading.** Die Befehle laufen im CDC-Task (Core 0), die UI im LVGL-Task (Core 1). Regeln, damit es keinen Deadlock gibt: (1) Datei-Zugriffe unter dem Profil-Mutex, danach loslassen; (2) erst *dann* `LvLock` für die UI; nie umgekehrt. (3) Das aktive Profil im RAM (`s_cur`) wird nur im LVGL-Task neu geladen: ein `PUT` auf das aktive Profil schreibt nur die Datei, `deckUiProfilesChanged()` lädt es unter `LvLock` neu.

**Programm starten.** Ein HID-Gerät kann kein Programm direkt starten. `run` nutzt, was Windows ohnehin anbietet: Win+R, Text tippen, Enter (700 ms Wartezeit, bis der Dialog offen ist). Das funktioniert für Programmnamen, Pfade und Links. Grenzen: kostet knapp eine Sekunde, braucht ein freies Eingabefenster, und wegen des deutschen Tastaturlayouts beim Tippen müssen Pfadzeichen zum Windows-Layout passen. Für präzisere Steuerung (Fenster aktivieren, Programm nur starten, wenn es nicht läuft) bleibt der Typ `host` mit einem späteren PC-Helfer.

**GitHub Pages.** Das Repo ist öffentlich, daher liefert Pages (Quelle `main`, Ordner `/docs`) den Editor über https: Web Serial verlangt einen sicheren Kontext, das ist bei einer lokal geöffneten Datei nicht garantiert. `docs/index.html` ist eine Kopie von `tools/editor/deckpad-editor.html` (`tools/deploy_editor.sh`), die Pages-Quelle muss ein Ordner im Repo sein.

**Web Serial.** `port.open` mit 115200 Baud, danach DTR/RTS setzen: `usbCdcSendLine` sendet nur, wenn die Gegenseite DTR gesetzt hat (`DeckSerial` ist dann „true“). Der Editor wurde im Demo-Modus (Gerätesimulator im Browser) getestet; am echten Gerät steht der Test noch aus.

---

## 2026-10-13 – v0.1.1: Streifen am linken Rand, Log

Erster Test von v0.1.0 am Gerät: Streifen und zerrissene Zeilen am linken Rand der Kacheln, wie im WetterCubePlus XL (dort PSRAM-Bandbreite und Zurückschreiben des Caches beim Flush, siehe XL-DEVLOG). Die damaligen Gegenmaßnahmen sind übernommen (4-Zeilen-Stücke, 15 MHz, Bounce 12 Zeilen, 32-px-Ausrichtung) und reichen im DeckPad offenbar nicht. Neu gegenüber dem XL sind: Flash-Zugriffe durch LittleFS (Formatieren und Schreiben stoppen den Cache, auch für den PSRAM), TinyUSB statt HWCDC, die 350-ms-Farbübergänge der Kacheln (viele kleine Flushes) und ein anderes Lastprofil beim Aufbau von 15 abgerundeten Kacheln. Welcher Punkt es ist, ist nicht bekannt.

Statt auf Verdacht an mehreren Stellen zugleich zu ändern (im XL ließ sich danach nicht mehr sagen, was gewirkt hat), wurde zuerst Messung eingebaut: Ringpuffer-Log (`LOG`), `STATUS` und eine Flush-Statistik. Nächster Schritt: Werte vom Gerät lesen, dann eine Maßnahme nach der anderen testen.

---

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
