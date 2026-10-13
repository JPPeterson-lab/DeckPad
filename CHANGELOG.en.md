# Changelog

*[Deutsche Version](CHANGELOG.md)*

## v0.3.3 (2026-10-13)

### New
- **Panel restart:** after every flash write the device also restarts the display drive (`esp_lcd_rgb_panel_restart`, per Espressif's documentation against a permanent shift when the LCD controller and DMA get out of sync) and redraws the screen. Plus COM command `PANEL` and an editor button **"Panel neu starten"** for testing. Whether this removes the streaks is not yet confirmed

## v0.3.2 (2026-10-13)

### New
- **Redraw display:** button in the editor and COM command `REDRAW`. Redraws the whole display. If a streak is gone afterwards, only the image content was disturbed; if it stays, the display drive itself is at fault. Used to narrow down the streaks at the left edge
- **Automatic redraw after flash writes** (saving a profile, uploading an icon, saving the selection): the RGB driver in the precompiled core is not IRAM-safe (`CONFIG_LCD_RGB_ISR_IRAM_SAFE` not set), so the drive pauses briefly during writes. Afterwards the whole screen is redrawn once; in the log as `[UI] Vollbild neu gezeichnet`

## v0.3.1 (2026-10-13)

### New
- **"Tasten am PC ausführen" (run keys on the PC):** second switch on the settings screen (default: off). Then the board does not act as a USB keyboard but sends every action as `DO <type> <spec>` over the COM port, and the **Windows helper** performs keys, media keys, text and program launches (`SendInput`, `Start-Process`). Meant for boards without a USB keyboard (e.g. the planned ESP32-P4 board) and as a fallback. Without the helper nothing happens (note in the log and on the display)
- The helper now listens to the device every 40 ms (previously only 0.6 s); the foreground window is still checked every 0.6 s

### Fixed
- **Shortcuts with Z or Y were swapped on German keyboards:** letters and digits were sent as the *physical* key of the US layout; on a German layout the Z position is the Y (`ctrl+z` arrived as Ctrl+Y). Letters and digits now follow the PC's layout (`DECK_KEY_LAYOUT`, default German). Affects e.g. the example profile "Bambu Studio" (undo). Ctrl+Shift+M, Ctrl+C/V, Ctrl+S were not affected. With a US-layout PC set `DECK_KEY_LAYOUT` to `KeyboardLayout_en_US`

## v0.3.0 (2026-10-13)

### New
- **Icons on the tiles:** symbols (emoji picker) or your own images (PNG, JPG, SVG …), converted to 32–80 px in the editor and loaded onto the device over the COM port (RGB565 + alpha, `/icons/<id>.bin`, up to 96 × 96 px). Several profiles can share an icon. The quick-pick in the editor brings matching symbols. COM commands `ICONBEGIN`, `ICONDATA`, `ICONEND`, `ICONLIST`, `ICONGET`, `ICONDEL`
- **Automatic profile switch (switchable):** switch "Automatischer Profilwechsel" on the settings screen (default: off). A small **Windows helper** (`tools/helper/deckpad-helper.ps1`, no installation) reports the foreground program; the DeckPad picks the profile whose "Programm" field matches (e.g. `bambu-studio.exe`), otherwise "Standard". The settings screen shows whether the helper is detected. COM command `ACTIVE <program.exe>`
- **Editor:** a **"Trennen"** (disconnect) button releases the COM port (the helper also releases it by itself while the editor is in the foreground)
- **Status display:** "PC verbunden" has a green background, "Kein PC" a red one

## v0.2.1 (2026-10-13)

### Changed
- **Profile selection is saved with a delay:** every profile switch used to write to flash immediately. Flash writes briefly stop the cache (and with it the display's PSRAM access) and wear the flash. The selection is now only written once nothing was switched for 8 s (`[Flash]` line in the log). Possible cause of the streaks after several profile switches, not yet confirmed

### New
- **More measurements in the log:** duration of the tile rebuild on every profile switch, and individual slow flushes (longer than 20 ms) with size and position of the area

## v0.2.0 (2026-10-13)

### New
- **DeckPad Editor** (`tools/editor/deckpad-editor.html`): create, edit, copy, delete and activate profiles on the **Windows PC** in the browser (Chrome/Edge, Web Serial). Click tiles, set label, **colour** (picker and palette), **action** (media, key combination with modifier boxes and key capture, start a program, text, host message) and quick-pick templates. Back up a profile as a file, demo mode without a device. Sets the PC's **time** on connect. Shows status and device log
- **Editor online:** available via GitHub Pages (<https://jppeterson-lab.github.io/DeckPad/>), nothing to install. `tools/deploy_editor.sh` copies the file to `docs/index.html`
- **Editor recognises the wrong port:** the chooser only lists Espressif devices (VID 0x303A), i.e. the native USB port; the UART port (CH343) is recognised and explained. "Alle Ports anzeigen" (show all ports) removes the filter
- **Action `run`:** starts a program, file or link via Win+R (program name, path, web address). The device presses Win+R, types the text and confirms with Enter
- **COM commands:** `VER`, `LIST`, `GET <id>`, `PUT <id> <json>`, `DEL <id>`, `USE <id>`, `TIME <unix seconds>` (in addition to `PING`, `STATUS`, `LOG`). The device applies changes immediately (roller, grid, colours)
- **Layers:** the USB layer only knows the channel; commands are registered by `app/host_commands.cpp`

### Changed
- COM line length from 256 to 6144 bytes, CDC task stack to 8 KB (a profile as JSON needs up to about 4 KB)

## v0.1.1 (2026-10-13)

### New
- **Diagnostic log:** ring buffer with the last 64 events (start-up incl. reset reason and display settings, USB events, key presses, profile switches, **flash writes with duration**). Every line also goes to the UART port
- **COM commands `STATUS` and `LOG`:** `STATUS` returns one line with version, uptime, free and minimum memory (SRAM, PSRAM) and the flush times; `LOG` prints the ring buffer
- **Flush measurement:** count, average and maximum duration of copying into the PSRAM framebuffer; a `[Diag]` line on the UART port every 10 s. Purpose: narrow down streaks and torn lines (PSRAM bandwidth) with measurements instead of guesses

## v0.1.0 (2026-10-13)

First skeleton. The code compiles; a test on the device is still pending.

### New
- **Project:** standalone project based on the WetterCubePlus XL (drivers for RGB panel, GT911, I²C, IO expander, LVGL partial rendering, `build_opt.h`). The WetterCubePlus XL itself was left untouched
- **Tile grid:** tiles are created in code inside the `ContainerGrid` placeholder of the PicoPixel design. Default 5 × 3, up to 6 × 4 per profile. On touch the tile changes colour immediately and fades back within 350 ms
- **USB to the PC:** native USB port as a composite device with HID keyboard, consumer/media keys and a virtual COM port. `Serial` stays on the USB-C UART port (flashing and debug)
- **USB/CAN switch:** IO5 of the IO expander is set explicitly to USB after the display init (the XL driver would have left it on CAN)
- **CDC:** line-based command reader with a command table; for now only `PING` is answered with `PONG`
- **Actions:** `media`, `keys` (e.g. `ctrl+shift+m`), `text`, `host`; execution runs in its own task so LVGL never waits for USB
- **Profiles:** one JSON file per program on the LittleFS partition with its own tiles and settings (theme, brightness, grid). Example profiles "Standard" and "Bambu Studio" on first start. Selected via the roller in the settings and the arrow keys on the deck
- **Per-profile settings:** theme and brightness are saved to the active profile as you change them (brightness on release, to spare the flash)
- **Calculator** from the WetterCubePlus XL, opened via the calculator icon in the settings
- **Kitchen timer and stopwatch:** the timer keeps running in the background; when it ends the display blinks (screen and backlight) on any screen until it is tapped (after one minute at the latest). Stopwatch with lap times
- **Partition table without OTA:** 4 MB factory app, 24 KB NVS, 64 KB coredump, 11.9 MB LittleFS
- **Tools:** `tools/deploy_ui.sh` (prepare and check the PicoPixel export), `tools/bump_version.sh`
- **Docs:** README, changelog and devlog in two languages, wiki

### Changed compared to the PicoPixel design
- The 15 static tile containers on the deck screen were replaced by **one** placeholder container `ContainerGrid` (27, 87, 950 × 384). The old containers 1–6 and 9–17 still have to be deleted in PicoPixel; `deploy_ui.sh` reports them as long as they exist

### Known
- The connection event on the device is not yet confirmed (the polarity of IO5, `0` = USB, is confirmed by the Waveshare documentation)
- The time shows `--:--` until a later step sets it from the PC
