# Changelog

*[Deutsche Version](CHANGELOG.md)*

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
