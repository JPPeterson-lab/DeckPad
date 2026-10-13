#include "deck_ui.h"
#include <Arduino.h>
#include <time.h>
#include "app_config.h"
#include "profiles.h"
#include "deck_actions.h"
#include "navigation.h"
#include "../ui/ui.h"
#include "../ui/colors.h"
#include "../ui/fonts/fonts.h"
#include "../usb/usb_device.h"
#include "../hardware/backlight.h"

// ── Gemeinsame Styles (ein Satz fuer alle Kacheln statt Einzel-Styles: spart LVGL-Speicher) ──
static lv_style_t                 stTile, stTilePressed;
static lv_style_transition_dsc_t  trFade, trInstant;
static bool                       stylesReady = false;

static void initStyles()
{
    if (stylesReady) return;
    static const lv_style_prop_t props[] = { LV_STYLE_BG_COLOR, (lv_style_prop_t)0 };
    // Loslassen: die Farbe blendet in 350 ms zurueck ("Taste ändert kurz die Farbe").
    lv_style_transition_dsc_init(&trFade, props, lv_anim_path_ease_out, 350, 0, nullptr);
    // Druecken: sofort umschalten, keine Verzoegerung
    lv_style_transition_dsc_init(&trInstant, props, lv_anim_path_linear, 0, 0, nullptr);

    lv_style_init(&stTile);
    lv_style_set_radius(&stTile, 16);
    lv_style_set_border_width(&stTile, 0);
    lv_style_set_shadow_width(&stTile, 0);
    lv_style_set_outline_width(&stTile, 0);
    lv_style_set_bg_opa(&stTile, LV_OPA_COVER);
    lv_style_set_pad_all(&stTile, 6);
    lv_style_set_text_color(&stTile, lv_color_white());
    lv_style_set_text_font(&stTile, &font_montserrat_22);
    lv_style_set_text_align(&stTile, LV_TEXT_ALIGN_CENTER);
    lv_style_set_transition(&stTile, &trFade);

    lv_style_init(&stTilePressed);
    lv_style_set_transition(&stTilePressed, &trInstant);
    stylesReady = true;
}

// ── Kachelraster ──────────────────────────────────────────────────────────────
static void cbTile(lv_event_t* e)
{
    const DeckProfile& p = profileActive();
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (i < 0 || i >= p.tileCount) return;
    const DeckTile& t = p.tiles[i];
    deckActionRun(t.type, t.spec);          // sofort beim Berühren, nicht erst beim Loslassen: fühlt sich direkter an
}

static void buildGrid()
{
    lv_obj_t* g = objects.containergrid;
    if (!g) return;
    lv_obj_clean(g);

    const DeckProfile& p = profileActive();
    const int gap = DECK_GRID_GAP;
    const int W = lv_obj_get_width(g), H = lv_obj_get_height(g);
    const int tw = (W - gap * (p.cols - 1)) / p.cols;
    const int th = (H - gap * (p.rows - 1)) / p.rows;
    const lv_color_t emptyColor = lv_color_hex(theme_colors[active_theme_index][THEME_COLOR_BACKGR_CONT] & 0xFFFFFF);

    for (int r = 0; r < p.rows; r++) {
        for (int c = 0; c < p.cols; c++) {
            const int i = r * p.cols + c;
            const int x = c * (tw + gap), y = r * (th + gap);
            if (i >= p.tileCount) {                         // freier Platz im Raster: dezente, nicht antippbare Fläche
                lv_obj_t* o = lv_obj_create(g);
                lv_obj_set_pos(o, x, y);
                lv_obj_set_size(o, tw, th);
                lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_set_style_radius(o, 16, 0);
                lv_obj_set_style_border_width(o, 0, 0);
                lv_obj_set_style_bg_color(o, emptyColor, 0);
                lv_obj_set_style_bg_opa(o, LV_OPA_40, 0);
                continue;
            }
            const DeckTile& t = p.tiles[i];
            lv_obj_t* b = lv_btn_create(g);
            lv_obj_set_pos(b, x, y);
            lv_obj_set_size(b, tw, th);
            lv_obj_add_style(b, &stTile, 0);
            lv_obj_add_style(b, &stTilePressed, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(b, lv_color_hex(t.color), 0);
            lv_obj_set_style_bg_color(b, lv_color_lighten(lv_color_hex(t.color), 110), LV_STATE_PRESSED);
            lv_obj_t* l = lv_label_create(b);
            lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(l, tw - 12);
            lv_label_set_text(l, t.label);
            lv_obj_center(l);
            lv_obj_add_event_cb(b, cbTile, LV_EVENT_PRESSED, (void*)(intptr_t)i);
        }
    }
}

// ── Profil uebernehmen ────────────────────────────────────────────────────────
void deckUiApplyProfile()
{
    const DeckProfile& p = profileActive();
    change_color_theme(p.theme);                            // PicoPixel: alle Farben des Themes
    backlightSet(p.brightness);
    lv_arc_set_value(objects.labellightchangevalue, p.brightness);
    lv_label_set_text(objects.labelprofil, p.name);
    lv_roller_set_selected(objects.rollerprofile, profileActiveIndex(), LV_ANIM_OFF);
    buildGrid();
}

static void switchProfile(int index)
{
    if (index == profileActiveIndex()) return;
    if (profileActivate(index)) deckUiApplyProfile();
    else Serial.printf("[UI] Profil %d konnte nicht geladen werden\n", index);
}

static void cbRoller(lv_event_t*) { switchProfile(lv_roller_get_selected(objects.rollerprofile)); }
static void cbPrev(lv_event_t*)   { int n = profileCount(); if (n > 1) switchProfile((profileActiveIndex() + n - 1) % n); }
static void cbNext(lv_event_t*)   { int n = profileCount(); if (n > 1) switchProfile((profileActiveIndex() + 1) % n); }

// ── Einstellungen: Helligkeit und Theme gelten pro Profil ──────────────────────
static void cbBrightness(lv_event_t* e)
{
    int v = lv_arc_get_value(objects.labellightchangevalue);
    if (v < DECK_BRIGHTNESS_MIN) v = DECK_BRIGHTNESS_MIN;
    backlightSet(v);                                        // live mitziehen
    if (lv_event_get_code(e) == LV_EVENT_RELEASED)          // erst beim Loslassen speichern (schont den Flash)
        profileSaveActiveSettings((uint8_t)active_theme_index, (uint8_t)v);
}

static void cbTheme(lv_event_t*)
{
    // PicoPixel hat das Theme in seinem eigenen Callback bereits umgeschaltet (zuerst registriert).
    profileSaveActiveSettings((uint8_t)active_theme_index, backlightGet());
    buildGrid();                                            // leere Plätze folgen der Theme-Farbe
}

// ── Kopfzeile: Uhr und PC-Status ──────────────────────────────────────────────
static void setIfChanged(lv_obj_t* l, const char* txt)
{
    if (strcmp(lv_label_get_text(l), txt) != 0) lv_label_set_text(l, txt);
}

static void tickCb(lv_timer_t*)
{
    char buf[8];
    time_t now = time(nullptr);
    if (now > 1700000000) {                                 // Zeit ist gesetzt (kommt spaeter vom PC)
        struct tm t; localtime_r(&now, &t);
        strftime(buf, sizeof(buf), "%H:%M", &t);
    } else {
        strcpy(buf, "--:--");
    }
    setIfChanged(objects.labeltime, buf);
    setIfChanged(objects.labelconnectionpc, usbHostConnected() ? "PC verbunden" : "Kein PC");
}

// ── Init ──────────────────────────────────────────────────────────────────────
void deckUiBootStatus(const char* text, int percent)
{
    lv_label_set_text(objects.labelstatus, text);
    lv_bar_set_value(objects.barwifi, percent, LV_ANIM_OFF);
}

void deckUiInit()
{
    initStyles();

    // Der Container aus PicoPixel ist nur ein Platzhalter: unsichtbar machen, Kacheln entstehen im Code
    lv_obj_t* g = objects.containergrid;
    lv_obj_clear_flag(g, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(g, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g, 0, 0);
    lv_obj_set_style_outline_width(g, 0, 0);
    lv_obj_set_style_shadow_width(g, 0, 0);
    lv_obj_set_style_radius(g, 0, 0);
    lv_obj_set_style_pad_all(g, 0, 0);

    // Profil-Roller mit den gespeicherten Profilnamen fuellen
    String names;
    profileNamesJoined(names);
    lv_roller_set_options(objects.rollerprofile, names.c_str(), LV_ROLLER_MODE_NORMAL);

    lv_obj_add_event_cb(objects.rollerprofile, cbRoller, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(objects.labelbuttonbackward, cbPrev, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(objects.labelbuttonforward,  cbNext, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(objects.labellightchangevalue, cbBrightness, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(objects.labellightchangevalue, cbBrightness, LV_EVENT_RELEASED, nullptr);
    lv_obj_add_event_cb(objects.labelswitchtheme, cbTheme, LV_EVENT_CLICKED, nullptr);

    lv_label_set_text(objects.labelversion, "v" FIRMWARE_VERSION);

    deckUiApplyProfile();
    lv_timer_create(tickCb, 1000, nullptr);
    tickCb(nullptr);
}
