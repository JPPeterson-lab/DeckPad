# 🎛️ DeckPad

![Status](https://img.shields.io/badge/Status-Alpha-orange?style=flat-square)
![Version](https://img.shields.io/badge/Version-v0.3.1-blue?style=flat-square&cacheSeconds=1)
![Hardware](https://img.shields.io/badge/Hardware-Waveshare%20ESP32--S3--Touch--LCD--7B-red?style=flat-square&logo=espressif&logoColor=white)
![Display](https://img.shields.io/badge/Display-7%22%201024×600%20RGB-informational?style=flat-square)
![Lizenz](https://img.shields.io/badge/Lizenz-CC%20BY--NC%204.0-lightgrey?style=flat-square)
![Plattform](https://img.shields.io/badge/Platform-Arduino%20IDE-teal?style=flat-square&logo=arduino&logoColor=white)

**DeckPad** ist ein **Stream-Deck-artiges Touch-Pad** auf dem Waveshare ESP32-S3-Touch-LCD-7B (7", 1024×600, kapazitiver Touch). Die Kacheln auf dem Display lösen am Windows-PC Aktionen aus: Lautstärke, Medientasten, beliebige Tastenkombinationen. Der PC erkennt das Board über den nativen USB-Port als **Tastatur (inkl. Media-Tasten) plus COM-Port**. Alle Kacheln entstehen im Code aus **Profilen**, jedes Programm bekommt sein eigenes Profil mit eigenen Tasten und eigenen Einstellungen.

Hardwaretreiber, LVGL-Port und Toolchain stammen aus dem [WetterCubePlus XL](https://github.com/JPPeterson-lab/WetterCubePlus-XL) (gleiches Board). Das Projekt läuft vollständig in der **Arduino IDE**, ohne WLAN und ohne OTA.

> **🟠 Alpha (v0.3.1):** Der Code kompiliert und ist gegen die ESP32-Core-Version 3.3.12 geprüft, ein Test am Gerät steht noch aus. Das Gerüst enthält Display + Touch, Kachelraster, USB (HID + CDC), Profile im Flash, Taschenrechner und Küchentimer/Stoppuhr – siehe [Status](#-status).

---

## 📊 Status

| Baustein | Status |
|---|---|
| RGB-Display 1024×600 + GT911-Touch, LVGL-Teilrendering (aus dem WetterCubePlus XL) | 🧪 übernommen, am DeckPad ungetestet |
| Kachelraster (5×3 Standard, pro Profil bis 6×4), Farbwechsel beim Antippen | 🧪 gebaut, ungetestet |
| USB-Composite: HID-Tastatur + Consumer/Media + virtueller COM-Port | 🧪 gebaut, ungetestet |
| CDC-Kanal: `PING` → `PONG`, `STATUS`, `LOG` | 🧪 gebaut, ungetestet |
| Profile im Flash (LittleFS, JSON pro Profil), Auswahl per Roller und Pfeil-Tasten | 🧪 gebaut, ungetestet |
| Einstellungen pro Profil (Theme hell/dunkel, Helligkeit) | 🧪 gebaut, ungetestet |
| Taschenrechner (aus dem WetterCubePlus XL) | 🧪 portiert, ungetestet |
| Küchentimer + Stoppuhr, Display blinkt bei Ablauf | 🧪 gebaut, ungetestet |
| Eigene Partitionstabelle ohne OTA (4 MB App, 11,9 MB Daten) | ✅ geprüft (Tabelle gebaut und gelesen) |
| Profile am PC bearbeiten und per CDC übertragen (DeckPad Editor, Web Serial) | 🧪 gebaut, im Demo-Modus getestet, am Gerät ungetestet |
| Uhrzeit vom PC per CDC (setzt der Editor beim Verbinden) | 🧪 gebaut, ungetestet |
| Aktionen über den PC-Helfer statt USB-Tastatur (Schalter + PowerShell) | 🧪 gebaut, ungetestet |
| Icons auf Kacheln (Emoji oder eigene Bilder, Upload über den Editor) | 🧪 gebaut, ungetestet |
| Automatischer Profilwechsel nach aktivem Programm (Schalter + PC-Helfer, PowerShell) | 🧪 gebaut, ungetestet |
| Statusanzeige „PC verbunden“ grün / „Kein PC“ rot | 🧪 gebaut, ungetestet |

---

## 🎬 Screens

| Screen | Inhalt |
|---|---|
| **Deck** | Uhrzeit, PC-Status, Profilname, Kachelraster, Pfeile zum Profilwechsel, Zahnrad |
| **Einstellungen** | Helligkeit, Theme, Profil-Roller, Version, Schnellzugriff auf Rechner und Timer |
| **Rechner** | Vier Grundrechenarten, Prozent, Vorzeichen (öffnet sich über das Rechner-Symbol) |
| **Timer / Stoppuhr** | Küchentimer mit Zeittasten; Stoppuhr mit Rundenzeiten (öffnet sich über das Uhr-Symbol) |
| **Boot** | Statuszeile und Balken beim Start |

Navigation: Deck → Zahnrad → Einstellungen → Home zurück zum Deck. Rechner und Timer öffnen sich aus den Einstellungen und kehren dorthin zurück. Der Timer läuft im Hintergrund weiter; läuft er ab, **blinkt das ganze Display** (auch auf jedem anderen Screen), bis man es antippt.

---

## 🎹 Profile und Kacheln

Jedes Profil ist eine JSON-Datei `/profiles/<id>.json` auf der Datenpartition. Es enthält Name, Programm, Rasterbreite/-höhe, **eigene Einstellungen** (Theme, Helligkeit) und die **Kacheln**:

```json
{
  "v": 1,
  "name": "Bambu Studio",
  "exe": "bambu-studio.exe",
  "cols": 5, "rows": 3,
  "settings": { "theme": "dark", "brightness": 70 },
  "tiles": [
    { "label": "Speichern", "color": "#27ae60", "type": "keys",  "spec": "ctrl+s" },
    { "label": "Lauter",    "color": "#2980b9", "type": "media", "spec": "vol_up" }
  ]
}
```

| `type` | `spec` | Wirkung |
|---|---|---|
| `none` | – | nur Farbwechsel (jede Kachel kann zusätzlich ein `icon` mit Icon-ID haben) |
| `media` | `vol_up`, `vol_down`, `mute`, `play_pause`, `next`, `prev`, `stop`, `calculator`, `browser`, `mail` | Medien-/Consumer-Taste |
| `keys` | z. B. `ctrl+shift+m` (Teile mit `+`) | Tastenkombination; Modifier `ctrl shift alt win altgr`, Tasten `a-z 0-9 f1-f24 enter esc tab space backspace delete insert home end pageup pagedown up down left right printscreen pause capslock` |
| `text` | beliebiger Text | wird getippt (deutsches Layout) |
| `run` | `notepad`, `C:\Pfad\x.exe`, `https://…` | startet Programm/Datei/Link über Win+R |
| `host` | frei | geht als `ACTION <spec>` über die COM-Schnittstelle an ein PC-Programm |

Beim ersten Start legt das Gerät zwei **Beispielprofile** an: **Standard** (Lauter, Leiser, Strg+Shift+M, Stumm, Play/Pause, Zurück, Weiter + Test-Kacheln) und **Bambu Studio** (Beispiel für ein Programm-Profil mit dunklem Theme). Gewechselt wird über den **Roller** in den Einstellungen oder die **Pfeil-Tasten** auf dem Deck. Theme und Helligkeit gelten **pro Profil** und werden beim Verstellen automatisch im aktiven Profil gespeichert.

---

## 🖥️ Profile am PC erstellen (DeckPad Editor)

Auf dem Windows-PC in **Chrome oder Edge** <https://jppeterson-lab.github.io/DeckPad/> öffnen (nichts zu installieren; die Seite ist `tools/editor/deckpad-editor.html`, auch als [Einzeldatei zum Speichern](https://raw.githubusercontent.com/JPPeterson-lab/DeckPad/main/tools/editor/deckpad-editor.html) nutzbar) und **„Mit DeckPad verbinden“** wählen (das Board muss über den USB-Port angeschlossen sein, der COM-Port darf nicht in einem anderen Programm offen sein). Dort:
- Profil wählen oder **Neu** / **Kopie** anlegen, **Name**, Programm, Raster, Design und Helligkeit einstellen,
- eine **Kachel anklicken**: Beschriftung, **Farbe** (Farbwähler oder Palette), **Aktion**: Medien/Lautstärke, **Tastenkombination**, **Programm, Datei oder Link starten**, Text tippen,
- **„Auf DeckPad speichern“**: Das Gerät übernimmt das Profil sofort. **„Auf dem Gerät aktivieren“** schaltet es um.

Der Editor setzt beim Verbinden außerdem die **Uhrzeit** des PCs und zeigt Status und Geräte-Log. Ohne Gerät lässt er sich mit **„Demo ohne Gerät“** ausprobieren. Als lokal gespeicherte Datei kann Web Serial je nach Browser blockiert sein; dann `python -m http.server` im Ordner `tools/editor` starten und `http://localhost:8000/deckpad-editor.html` öffnen.

**Programm starten** (`run`): Das DeckPad drückt Win+R, tippt den Text und bestätigt mit Enter, geeignet sind Programmname (`notepad`, `calc`), vollständiger Pfad oder Internetadresse.

### Icons auf Kacheln
Im Editor bei einer Kachel **„Symbol wählen“** (Emoji) oder **„Bild laden …“** (PNG, JPG, SVG). Die Größe (32–80 px) wählst du daneben; mit Text darunter besser 48 px oder kleiner. Beim Speichern lädt der Editor neue Icons ins Gerät (dort bis 96 × 96 px, gemeinsam für alle Profile).

## 🔄 Automatischer Profilwechsel
Das DeckPad kann das Profil nach dem Programm im Vordergrund wählen. Einschalten: **Einstellungen → „Automatischer Profilwechsel“** (Standard: aus). Dazu braucht der PC den kleinen **Helfer**:
1. [`deckpad-helper.ps1`](https://jppeterson-lab.github.io/DeckPad/deckpad-helper.ps1) und [`start-helper.cmd`](https://jppeterson-lab.github.io/DeckPad/start-helper.cmd) in denselben Ordner speichern (Rechtsklick → *Link speichern unter*; ggf. Eigenschaften → *Zulassen*).
2. `start-helper.cmd` doppelklicken; für den Autostart eine Verknüpfung darauf in den Ordner `shell:startup` legen.
3. Im Profil das Feld **Programm** ausfüllen (z. B. `bambu-studio.exe`, den Namen zeigt der Task-Manager). Für Programme ohne eigenes Profil gilt **Standard**.

Der Einstellungs-Screen zeigt, ob der Helfer erkannt wird. Ein COM-Port gehört immer nur einem Programm: Der Helfer gibt ihn frei, solange der Editor im Vordergrund ist; bleibt der Editor verbunden, bitte **„Trennen“** klicken. Der Helfer sendet nur den Namen des Vordergrundprogramms (z. B. `chrome.exe`) an das DeckPad.

### Tasten am PC ausführen (ohne USB-Tastatur)
Zweiter Schalter in den Einstellungen: **„Tasten am PC ausführen“** (Standard: aus). Dann sendet das Board die Aktionen über den COM-Port, und **derselbe Helfer** führt sie aus (Tasten, Medientasten, Text, Programmstart). Das ist für Boards gedacht, die sich nicht als USB-Tastatur anmelden können, und als Ausweichweg. Ohne laufenden Helfer passiert beim Antippen nichts. In diesem Modus liest der Helfer auch, was das Gerät schickt, und führt es aus; deshalb nur Helfer laufen lassen, die du kennst. Tastaturlayout: Buchstaben und Ziffern in Kürzeln folgen dem Layout des PCs (Standard Deutsch, einstellbar über `DECK_KEY_LAYOUT` in `app_config.h`).

## 🔌 Hardware

| Bauteil | Details |
|---|---|
| Board | [Waveshare ESP32-S3-Touch-LCD-7B](https://www.waveshare.com/esp32-s3-touch-lcd-7b.htm) – ESP32-S3, 16 MB Flash, 8 MB PSRAM (OPI) |
| Display | 7", 1024×600, 16-Bit-RGB-Interface |
| Touch | GT911 (I²C), kapazitiv |
| Backlight | PWM über den IO-Expander (I²C) |
| Zwei USB-C-Buchsen | **UART** (CH343): Flashen + Debug-Ausgabe · **USB** (nativ, GPIO19/20): Verbindung zum PC |

### ⚠️ USB-Pins GPIO19/20 teilen sich USB und CAN
Das Board führt USB und den CAN-Transceiver auf **dieselben Pins GPIO19/20**. Über **IO5 des IO-Expanders** wird umgeschaltet (laut Waveshare-Header: `0` = USB, `1` = CAN). Die RGB-Panel-Pins überschneiden sich nicht mit GPIO19/20, USB ist also nutzbar – **aber**: Der Expander merkt sich alle acht Ausgänge, und der Treiber aus dem WetterCubePlus XL schreibt sie bei jedem Zugriff (Touch-Reset, Backlight). Wird IO5 nie gesetzt, steht er nach dem Start auf `1` (= CAN) und der native USB-Port bleibt stumm. Das DeckPad setzt IO5 deshalb nach der Display-/Touch-Initialisierung explizit (`DECK_USB_SELECT_LEVEL` in `src/app/app_config.h`).
Konsequenzen: **CAN ist im DeckPad nicht nutzbar**. Die Polarität (`0` = USB) bestätigt die [Waveshare-Dokumentation](https://docs.waveshare.com/ESP32-S3-Touch-LCD-7B) („EXIO5 low = USB-Modus“).

---

## 💾 Speicher

| Bereich | Größe | Inhalt |
|---|---|---|
| `nvs` | 24 KB | Systemdaten |
| `factory` | 4 MB | Firmware (derzeit ca. 1 MB) – **kein OTA**, keine zweite App-Partition |
| `coredump` | 64 KB | Absturzprotokoll für die Fehlersuche |
| `storage` (LittleFS) | 11,9 MB | Profile, später Icons und Bilder |
| PSRAM (8 MB) | – | Framebuffer (1,2 MB) und LVGL-Speicherpool (192 KB); wird nicht partitioniert |

---

## 🛠️ Installation (Arduino IDE / arduino-cli)

1. **ESP32-Core** (≥ 3.3) sowie die Bibliotheken **LVGL 8.x** und **ArduinoJson 7.x** installieren. Die gemeinsame `lv_conf.h` liegt in `Arduino/libraries/` neben dem Ordner `lvgl` – **`LV_COLOR_16_SWAP` muss `0` sein**.
2. Ordner öffnen, **Board: ESP32S3 Dev Module** mit:

| Menü | Wert |
|---|---|
| PSRAM | OPI PSRAM |
| Flash Size / Mode | 16 MB / QIO |
| Partition Scheme | Custom (die `partitions.csv` im Sketch-Ordner wird verwendet) |
| **USB Mode** | **USB-OTG (TinyUSB)** |
| **USB CDC On Boot** | **Disabled** (damit `Serial` auf dem UART-Port bleibt) |

3. Mit dem **UART-Port** des Boards verbinden und flashen. Danach das Board **zusätzlich über den USB-Port** (nativ) mit dem PC verbinden.

Kommandozeile:
```bash
arduino-cli compile --fqbn "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,FlashMode=qio,PartitionScheme=custom,USBMode=default,CDCOnBoot=default" .
arduino-cli upload  --fqbn "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,FlashMode=qio,PartitionScheme=custom,USBMode=default,CDCOnBoot=default" -p <UART-Port> .
```

### Testen
- Tippe auf **Lauter / Leiser** – die Windows-Lautstärke ändert sich. Tippe auf **Strg+Shift+M** – im Programm mit dieser Tastenkombination (z. B. Teams) wird sie ausgelöst.
- Windows: Geräte-Manager → *Tastaturen* (HID-Tastatur) und *Anschlüsse (COM & LPT)* (USB-Seriell-Gerät).
- COM-Port mit einem Terminalprogramm (PuTTY, Tera Term, 115200 – die Baudrate ist egal) öffnen, `PING` + Enter senden → Antwort `PONG`.
- Debug-Ausgabe: serieller Monitor auf dem **UART-Port** (115200). Am Windows-PC lässt sich dieselbe Diagnose über den **COM-Port** abrufen: im DeckPad Editor „Status“ / „Geräte-Log“ oder in einem Terminal `STATUS` bzw. `LOG` senden.

### Neu-Export der Oberfläche
Die Oberfläche entsteht in [PicoPixel](https://picopixel.io) (LVGL 8, 1024×600, 16 Bit). Nach jedem Export im Projektordner `./tools/deploy_ui.sh` ausführen (benennt Bilder in `.inc` um, schaltet die Screen-Überblendung ab und prüft, ob alle verdrahteten Objekte noch existieren).

### Projektstruktur
```
Deckpad/
├─ Deckpad.ino              Einstieg (Reihenfolge des Starts)
├─ build_opt.h, partitions.csv
├─ tools/                   deploy_ui.sh, bump_version.sh
└─ src/
   ├─ hardware/   Waveshare-Treiber: RGB-Panel, GT911, I²C, IO-Expander, Backlight
   ├─ display/    LVGL-Anbindung (Teilrendering)
   ├─ usb/        USB-Gerät (HID + CDC) und CDC-Befehle – reine Hardware-Schicht
   ├─ app/        Profile, Aktionen, UI-Logik (Kacheln), Navigation, Rechner, Timer
   └─ ui/         PicoPixel-Export
```
Die UI-Logik (`deck_ui.cpp`) kennt nur LVGL und die Profile. Eine Kachel ruft `deckActionRun()` auf; erst `deck_actions.cpp` spricht mit `usb/`.

---

## 📚 Wiki
Ausführliche Seiten (Hardware, Inbetriebnahme, Profile, USB und CDC, Entwicklung): <https://github.com/JPPeterson-lab/DeckPad/wiki>

## 🙏 Credits
Siehe [CREDITS.md](CREDITS.md). GUI: [LVGL](https://lvgl.io). Oberfläche: [PicoPixel](https://picopixel.io). Treiber: Waveshare / Espressif.

## 📄 Lizenz
[CC BY-NC 4.0](LICENSE) – siehe Datei für Details und Drittkomponenten.

---
---

# 🎛️ DeckPad (English)

**DeckPad** is a **Stream-Deck-style touch pad** on the Waveshare ESP32-S3-Touch-LCD-7B (7", 1024×600, capacitive touch). The tiles on the display trigger actions on the Windows PC: volume, media keys, any key combination. The PC sees the board through its native USB port as a **keyboard (incl. media keys) plus a COM port**. All tiles are generated in code from **profiles**; every program gets its own profile with its own keys and its own settings.

Hardware drivers, the LVGL port and the toolchain come from the [WetterCubePlus XL](https://github.com/JPPeterson-lab/WetterCubePlus-XL) (same board). The project builds entirely in the **Arduino IDE**, with no Wi-Fi and no OTA.

> **🟠 Alpha (v0.3.1):** The code compiles and was checked against ESP32 core 3.3.12; it has **not been tested on the device yet**. The skeleton contains display + touch, tile grid, USB (HID + CDC), profiles in flash, calculator and kitchen timer/stopwatch – see [Status](#-status-1).

## 📊 Status

| Component | Status |
|---|---|
| 1024×600 RGB display + GT911 touch, LVGL partial rendering (from the WetterCubePlus XL) | 🧪 taken over, untested on the DeckPad |
| Tile grid (5×3 default, up to 6×4 per profile), colour change on touch | 🧪 built, untested |
| USB composite: HID keyboard + consumer/media + virtual COM port | 🧪 built, untested |
| CDC channel: `PING` → `PONG`, `STATUS`, `LOG` | 🧪 built, untested |
| Profiles in flash (LittleFS, one JSON per profile), selection by roller and arrow keys | 🧪 built, untested |
| Per-profile settings (theme light/dark, brightness) | 🧪 built, untested |
| Calculator (from the WetterCubePlus XL) | 🧪 ported, untested |
| Kitchen timer + stopwatch, display blinks when the timer ends | 🧪 built, untested |
| Custom partition table without OTA (4 MB app, 11.9 MB data) | ✅ verified (table built and read back) |
| Edit profiles on the PC and transfer them via CDC (DeckPad Editor, Web Serial) | 🧪 built, tested in demo mode, untested on the device |
| Time from the PC via CDC (the editor sets it on connect) | 🧪 built, untested |
| Actions via the PC helper instead of a USB keyboard (switch + PowerShell) | 🧪 built, untested |
| Icons on tiles (emoji or own images, upload via the editor) | 🧪 built, untested |
| Automatic profile switch by active program (switch + PC helper, PowerShell) | 🧪 built, untested |
| Status display "PC verbunden" green / "Kein PC" red | 🧪 built, untested |

## 🎬 Screens

| Screen | Content |
|---|---|
| **Deck** | Time, PC status, profile name, tile grid, arrows to switch profile, gear |
| **Settings** | Brightness, theme, profile roller, version, shortcuts to calculator and timer |
| **Calculator** | Four basic operations, percent, sign (opens via the calculator icon) |
| **Timer / stopwatch** | Kitchen timer with time keys; stopwatch with lap times (opens via the clock icon) |
| **Boot** | Status line and bar at start-up |

Navigation: deck → gear → settings → home back to the deck. Calculator and timer open from the settings and return there. The timer keeps running in the background; when it ends, **the whole display blinks** (on any screen) until you tap it.

## 🎹 Profiles and tiles

Each profile is a JSON file `/profiles/<id>.json` on the data partition: name, program, grid size, **its own settings** (theme, brightness) and the **tiles** (see the JSON example above).

| `type` | `spec` | Effect |
|---|---|---|
| `none` | – | colour change only (every tile may additionally have an `icon` with an icon id) |
| `media` | `vol_up`, `vol_down`, `mute`, `play_pause`, `next`, `prev`, `stop`, `calculator`, `browser`, `mail` | media / consumer key |
| `keys` | e.g. `ctrl+shift+m` (parts joined by `+`) | key combination; modifiers `ctrl shift alt win altgr`, keys `a-z 0-9 f1-f24 enter esc tab space backspace delete insert home end pageup pagedown up down left right printscreen pause capslock` |
| `text` | any text | typed out (German layout) |
| `run` | `notepad`, `C:\path\x.exe`, `https://…` | starts a program/file/link via Win+R |
| `host` | free | sent as `ACTION <spec>` over the COM port to a PC program |

On first start the device creates two **example profiles**: **Standard** (volume up/down, Ctrl+Shift+M, mute, play/pause, previous/next + test tiles) and **Bambu Studio** (example of a per-program profile with dark theme). Switch with the **roller** in the settings or the **arrow keys** on the deck. Theme and brightness are **per profile** and are saved to the active profile as you change them.

## 🖥️ Creating profiles on the PC (DeckPad Editor)

On the Windows PC open <https://jppeterson-lab.github.io/DeckPad/> in **Chrome or Edge** (nothing to install; the page is `tools/editor/deckpad-editor.html`, also usable as a [single file to save](https://raw.githubusercontent.com/JPPeterson-lab/DeckPad/main/tools/editor/deckpad-editor.html)) and choose **"Mit DeckPad verbinden"** (the board must be connected through the USB port; the COM port must not be open in another program). There you can:
- pick a profile or create **Neu** (new) / **Kopie** (copy), set **name**, program, grid, theme and brightness,
- **click a tile**: label, **colour** (picker or palette), **action**: media/volume, **key combination**, **start a program, file or link**, type text,
- **"Auf DeckPad speichern"** (save): the device applies the profile immediately. **"Auf dem Gerät aktivieren"** switches to it.

On connect the editor also sets the PC's **time** and shows status and device log. Without a device try **"Demo ohne Gerät"**. As a locally saved file Web Serial may be blocked depending on the browser; then run `python -m http.server` in the folder `tools/editor` and open `http://localhost:8000/deckpad-editor.html`.

**Start a program** (`run`): the DeckPad presses Win+R, types the text and confirms with Enter; program name (`notepad`, `calc`), full path or web address work.

### Icons on tiles
In the editor choose **"Symbol wählen"** (emoji) or **"Bild laden …"** (image: PNG, JPG, SVG) for a tile. Choose the size (32–80 px) next to it; with text underneath prefer 48 px or smaller. On save the editor loads new icons onto the device (up to 96 × 96 px there, shared by all profiles).

## 🔄 Automatic profile switch
The DeckPad can pick the profile by the program in the foreground. Enable it: **settings → "Automatischer Profilwechsel"** (default: off). The PC needs the small **helper** for this:
1. Save [`deckpad-helper.ps1`](https://jppeterson-lab.github.io/DeckPad/deckpad-helper.ps1) and [`start-helper.cmd`](https://jppeterson-lab.github.io/DeckPad/start-helper.cmd) into the same folder (right-click → *Save link as*; if needed properties → *Unblock*).
2. Double-click `start-helper.cmd`; for autostart put a shortcut to it into the folder `shell:startup`.
3. Fill the profile's **Programm** field (e.g. `bambu-studio.exe`, the Task Manager shows the name). Programs without their own profile get **Standard**.

The settings screen shows whether the helper is detected. A COM port can belong to one program only: the helper releases it while the editor is in the foreground; if the editor stays connected please click **"Trennen"**. The helper only sends the name of the foreground program (e.g. `chrome.exe`) to the DeckPad.

### Run keys on the PC (without a USB keyboard)
Second switch in the settings: **"Tasten am PC ausführen"** (default: off). Then the board sends the actions over the COM port and **the same helper** performs them (keys, media keys, text, program launch). This is meant for boards that cannot enumerate as a USB keyboard, and as a fallback. Without a running helper nothing happens when you tap. In this mode the helper reads what the device sends and executes it, so only run helpers you know. Keyboard layout: letters and digits in shortcuts follow the PC's layout (default German, set via `DECK_KEY_LAYOUT` in `app_config.h`).

## 🔌 Hardware

| Part | Details |
|---|---|
| Board | [Waveshare ESP32-S3-Touch-LCD-7B](https://www.waveshare.com/esp32-s3-touch-lcd-7b.htm) – ESP32-S3, 16 MB flash, 8 MB PSRAM (OPI) |
| Display | 7", 1024×600, 16-bit RGB interface |
| Touch | GT911 (I²C), capacitive |
| Backlight | PWM via the I²C IO expander |
| Two USB-C ports | **UART** (CH343): flashing + debug output · **USB** (native, GPIO19/20): link to the PC |

### ⚠️ GPIO19/20 are shared between USB and CAN
The board routes USB and the CAN transceiver to the **same pins GPIO19/20**. **IO5 of the IO expander** switches them (per the Waveshare header: `0` = USB, `1` = CAN). The RGB panel pins do not overlap with GPIO19/20, so USB is usable – **but** the expander remembers all eight outputs and the driver from the WetterCubePlus XL rewrites them on every access (touch reset, backlight). If IO5 is never set it ends up at `1` (= CAN) after start-up and the native USB port stays silent. The DeckPad therefore sets IO5 explicitly after the display/touch initialisation (`DECK_USB_SELECT_LEVEL` in `src/app/app_config.h`).
Consequence: **CAN cannot be used with the DeckPad**. The polarity (`0` = USB) is confirmed by the [Waveshare documentation](https://docs.waveshare.com/ESP32-S3-Touch-LCD-7B) ("EXIO5 low = USB mode").

## 💾 Storage

| Area | Size | Content |
|---|---|---|
| `nvs` | 24 KB | system data |
| `factory` | 4 MB | firmware (about 1 MB now) – **no OTA**, no second app partition |
| `coredump` | 64 KB | crash dump for debugging |
| `storage` (LittleFS) | 11.9 MB | profiles, later icons and images |
| PSRAM (8 MB) | – | framebuffer (1.2 MB) and LVGL memory pool (192 KB); not partitioned |

## 🛠️ Installation (Arduino IDE / arduino-cli)

1. Install the **ESP32 core** (≥ 3.3) and the libraries **LVGL 8.x** and **ArduinoJson 7.x**. The shared `lv_conf.h` lives in `Arduino/libraries/` next to the `lvgl` folder – **`LV_COLOR_16_SWAP` must be `0`**.
2. Open the folder, **board: ESP32S3 Dev Module** with the values from the German table above (PSRAM *OPI*, 16 MB / QIO, partition scheme *Custom*, **USB Mode: USB-OTG (TinyUSB)**, **USB CDC On Boot: Disabled** so that `Serial` stays on the UART port).
3. Connect to the board's **UART port** and flash. Afterwards also connect the **native USB port** to the PC.

The `arduino-cli` commands are shown in the German section.

### Testing
- Tap **Lauter / Leiser** (volume up/down) – the Windows volume changes. Tap **Strg+Shift+M** – the key combination fires in the program that listens for it (e.g. Teams).
- Windows: Device Manager → *Keyboards* (HID keyboard) and *Ports (COM & LPT)* (USB serial device).
- Open the COM port in a terminal program (PuTTY, Tera Term; the baud rate does not matter), send `PING` + Enter → reply `PONG`.
- Debug output: serial monitor on the **UART port** (115200). On the Windows PC the same diagnostics are available via the **COM port**: "Status" / "Geräte-Log" in the DeckPad Editor, or send `STATUS` / `LOG` in a terminal.

### Re-exporting the UI
The UI is designed in [PicoPixel](https://picopixel.io) (LVGL 8, 1024×600, 16 bit). After every export run `./tools/deploy_ui.sh` (renames images to `.inc`, disables the screen fade and checks that all wired objects still exist).

### Layers
`hardware/` (display, touch, backlight) → `usb/` (HID + CDC, hardware layer only) → `app/` (profiles, actions, UI logic) → `ui/` (PicoPixel export). The UI logic (`deck_ui.cpp`) only knows LVGL and profiles; a tile calls `deckActionRun()`, and only `deck_actions.cpp` talks to `usb/`.

## 📚 Wiki
Detailed pages (hardware, getting started, profiles, USB and CDC, development): <https://github.com/JPPeterson-lab/DeckPad/wiki>

## 🙏 Credits
See [CREDITS.md](CREDITS.md). GUI: [LVGL](https://lvgl.io). UI design: [PicoPixel](https://picopixel.io). Drivers: Waveshare / Espressif.

## 📄 License
[CC BY-NC 4.0](LICENSE) – see the file for details and third-party components.
