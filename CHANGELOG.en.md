# Changelog

*[Deutsche Version](CHANGELOG.md)*

## v0.2.0 (2026-10-13)

### New
- **DeckPad Editor** (`tools/editor/deckpad-editor.html`): create, edit, copy, delete and activate profiles on the **Windows PC** in the browser (Chrome/Edge, Web Serial). Click tiles, set label, **colour** (picker and palette), **action** (media, key combination with modifier boxes and key capture, start a program, text, host message) and quick-pick templates. Back up a profile as a file, demo mode without a device. Sets the PC's **time** on connect. Shows status and device log
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
- Polarity of IO5 (`0` = USB) and the connection event not yet confirmed on the device
- The time shows `--:--` until a later step sets it from the PC
