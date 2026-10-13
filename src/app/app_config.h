#pragma once
// Zentrale Einstellungen des DeckPad. Alles, was man später mal anpassen will, steht hier.

#define FIRMWARE_VERSION "0.3.1"

// ── USB-Geraet (so meldet sich das Board am Windows-PC an) ─────────────────────
#define DECK_USB_MANUFACTURER "JPPeterson-lab"
#define DECK_USB_PRODUCT      "DeckPad"
// IO5 des IO-Expanders schaltet die Pins GPIO19/20 zwischen USB und CAN um.
// Laut Waveshare-Header: 0 = USB, 1 = CAN. Falls der PC das Board nicht sieht: Wert tauschen.
#define DECK_USB_SELECT_LEVEL 0

// Tastaturlayout des Windows-PCs (fuer Text und Buchstaben in Tastenkombinationen): KeyboardLayout_de_DE oder KeyboardLayout_en_US
#define DECK_KEY_LAYOUT KeyboardLayout_de_DE

// ── Kachelraster ───────────────────────────────────────────────────────────────
#define DECK_DEFAULT_COLS 5     // Standardraster (wie im PicoPixel-Entwurf: 5 x 3 = 15 Tasten)
#define DECK_DEFAULT_ROWS 3
#define DECK_MAX_COLS     6     // Obergrenze pro Profil
#define DECK_MAX_ROWS     4
#define DECK_MAX_TILES    (DECK_MAX_COLS * DECK_MAX_ROWS)
#define DECK_GRID_GAP     16    // Abstand zwischen den Kacheln in Pixeln

// ── Profile im Flash (LittleFS auf der Partition "storage") ───────────────────
#define DECK_MAX_PROFILES   16
#define DECK_STORAGE_LABEL  "storage"
#define DECK_PROFILE_DIR    "/profiles"
#define DECK_CONFIG_FILE    "/config.json"

// ── Helligkeit ─────────────────────────────────────────────────────────────────
#define DECK_BRIGHTNESS_MIN 3
#define DECK_BRIGHTNESS_DEFAULT 80
