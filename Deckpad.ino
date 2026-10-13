// DeckPad — Stream-Deck-ähnliches Touch-Pad auf dem Waveshare ESP32-S3-Touch-LCD-7B (1024x600, RGB + GT911)
//
// Arduino-IDE-Einstellungen (Details im README):
//   Board: ESP32S3 Dev Module | PSRAM: OPI PSRAM | Flash Size: 16 MB | Flash Mode: QIO
//   Partition Scheme: Custom (partitions.csv im Sketch-Ordner) | USB Mode: USB-OTG (TinyUSB)
//   USB CDC On Boot: Disabled   -> "Serial" bleibt auf dem USB-C-UART-Port (Flashen + Debug),
//                                  der native USB-Port gehört dem PC (Tastatur + COM-Port)
//
// Schichten:  hardware/ (Display, Touch, Backlight)  usb/ (HID + CDC)  app/ (Profile, Aktionen, UI-Logik)
//             ui/ (PicoPixel-Export, nicht von Hand ändern)
#include <Arduino.h>
#include "src/app/app_config.h"
#include "src/display/lvgl_port.h"
#include "src/hardware/rgb_lcd_port.h"
#include "src/hardware/backlight.h"
#include "src/usb/usb_device.h"
#include "src/usb/usb_cdc.h"
#include "src/app/deck_actions.h"
#include "src/app/profiles.h"
#include "src/app/navigation.h"
#include "src/app/deck_ui.h"
#include "src/app/tools.h"
#include "src/app/lv_lock.h"
#include "src/app/host_commands.h"
#include "src/app/diag_log.h"
#include "esp_system.h"
#include "src/ui/ui.h"

// Statuszeile auf dem Bootscreen aktualisieren und LVGL kurz Zeit zum Zeichnen geben
static void boot(const char* text, int percent)
{
    logDiag("[Boot] %s", text);
    { LvLock lock; deckUiBootStatus(text, percent); }
    vTaskDelay(pdMS_TO_TICKS(120));
}

void setup()
{
    Serial.begin(115200);                       // UART0 -> USB-C-UART-Port des Boards
    Serial.println();
    logDiag("[DeckPad] Start v" FIRMWARE_VERSION ", Reset-Grund %d, CPU %u MHz, PSRAM %u KB", (int)esp_reset_reason(),
            (unsigned)getCpuFrequencyMhz(), (unsigned)(ESP.getPsramSize() / 1024));
    logDiag("[Display] Pixeltakt %u MHz, Bounce %d Zeilen, Zeichenpuffer %d Zeilen (SRAM), Flush-Stuecke %d Zeilen, Ausrichtung %d px",
            (unsigned)(EXAMPLE_LCD_PIXEL_CLOCK_HZ / 1000000), (int)(EXAMPLE_RGB_BOUNCE_BUFFER_SIZE / EXAMPLE_LCD_H_RES),
            LVGL_PORT_BUF_LINES, LVGL_PORT_FLUSH_CHUNK_LINES, LVGL_PORT_ALIGN_PX);
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);   // Mitteleuropäische Zeit; die Uhrzeit selbst kommt später vom PC
    tzset();

    // ── Display und Touch (unverändert aus dem WetterCubePlus XL) ──
    esp_lcd_touch_handle_t tp    = touch_gt911_init();             // initialisiert auch I2C + IO-Expander
    esp_lcd_panel_handle_t panel = waveshare_esp32_s3_rgb_lcd_init();

    void *fb = nullptr;                                            // Framebuffer schwarz vorbelegen
    waveshare_get_frame_buffer(&fb, nullptr);
    if (fb) memset(fb, 0, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2);

    wavesahre_rgb_lcd_bl_on();
    backlightInit();
    ESP_ERROR_CHECK(lvgl_port_init(panel, tp));

    if (lvgl_port_lock(-1)) {
        ui_init();                              // erzeugt die PicoPixel-Screens
        nav_init();
        nav_goto(SCREEN_ID_SCREENBOOT);
        deckUiBootStatus("Starte...", 5);
        lvgl_port_unlock();
    }

    // ── USB zum PC ──
    boot("Starte USB...", 25);
    usbDeviceInit();                            // muss nach der IO-Expander-Init laufen (IO5 = USB)
    usbCdcInit();
    deckActionsInit();

    // ── Profile aus dem Flash ──
    boot("Lade Profile...", 55);
    bool storageOk = profilesInit();
    hostCommandsInit();                         // COM-Befehle: Profile, Uhrzeit, Diagnose

    // ── Oberfläche mit Profilen verdrahten ──
    boot(storageOk ? "Baue Oberfläche..." : "Speicher nicht lesbar - Notprofil", 80);
    if (lvgl_port_lock(-1)) {
        deckUiInit();
        uiRechnerInit();
        uiTimerInit();
        lvgl_port_unlock();
    }

    boot("Bereit", 100);
    vTaskDelay(pdMS_TO_TICKS(500));
    if (lvgl_port_lock(-1)) { nav_goto(SCREEN_ID_SCREEN_1); lvgl_port_unlock(); }

    logDiag("[Mem] nach dem Start: SRAM frei %u (groesster Block %u) | PSRAM frei %u",
            (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
            (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void loop()
{
    vTaskDelay(portMAX_DELAY);   // alles läuft in Tasks: LVGL (Core 1), USB/Aktionen (Core 0)
}
