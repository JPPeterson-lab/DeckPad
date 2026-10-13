#include "settings_extras.h"
#include <Arduino.h>
#include "profiles.h"
#include "../ui/ui.h"
#include "../ui/colors.h"
#include "../ui/fonts/fonts.h"

static lv_obj_t *s_box, *s_title, *s_sw, *s_state, *s_hint;
static volatile uint32_t s_helperMs = 0;     // Zeitpunkt der letzten Meldung des PC-Helfers (0 = noch nie)
#define HELPER_TIMEOUT_MS 10000

void settingsExtrasHelperSeen() { s_helperMs = millis() ? millis() : 1; }

static lv_color_t themeColor(int idx) { return lv_color_hex(theme_colors[active_theme_index][idx] & 0xFFFFFF); }

void settingsExtrasApplyTheme()
{
    if (!s_box) return;
    lv_obj_set_style_bg_color(s_box, themeColor(THEME_COLOR_BACKGR_CONT), 0);
    lv_obj_set_style_text_color(s_title, themeColor(THEME_COLOR_TEXT), 0);
    lv_obj_set_style_text_color(s_state, themeColor(THEME_COLOR_TEXT), 0);
    lv_obj_set_style_text_color(s_hint, themeColor(THEME_COLOR_TEXT), 0);
    lv_obj_set_style_bg_color(s_sw, themeColor(THEME_COLOR_HIGHLITE_2), LV_PART_INDICATOR | LV_STATE_CHECKED);
}

static void cbSwitch(lv_event_t*)
{
    profilesSetAutoSwitch(lv_obj_has_state(s_sw, LV_STATE_CHECKED));
    settingsExtrasTick();
}

static lv_obj_t* mkLabel(const lv_font_t* f, int x, int y, int w, const char* text)
{
    lv_obj_t* l = lv_label_create(s_box);
    lv_obj_set_pos(l, x, y);
    if (w) { lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP); lv_obj_set_width(l, w); }
    lv_obj_set_style_text_font(l, f, 0);
    lv_label_set_text(l, text);
    return l;
}

void settingsExtrasInit()
{
    s_box = lv_obj_create(objects.screensetting);
    lv_obj_set_pos(s_box, 10, 290);
    lv_obj_set_size(s_box, 970, 150);
    lv_obj_clear_flag(s_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(s_box, 5, 0);
    lv_obj_set_style_pad_all(s_box, 0, 0);
    lv_obj_set_style_border_width(s_box, 1, 0);          // wie die Container darueber: schwarzer Rand 1 px
    lv_obj_set_style_border_color(s_box, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_box, LV_OPA_COVER, 0);

    s_title = mkLabel(&font_montserrat_22, 20, 18, 0, "Automatischer Profilwechsel");
    s_state = mkLabel(&font_montserrat_18, 20, 62, 0, "");
    s_hint  = mkLabel(&font_montserrat_18, 20, 92, 930,
                      "Wechselt das Profil nach dem Programm im Vordergrund (Feld \"Programm\" im Profil, z. B. bambu-studio.exe). "
                      "Ohne passendes Profil gilt \"Standard\". Braucht den DeckPad-Helfer auf dem PC.");

    s_sw = lv_switch_create(s_box);
    lv_obj_set_size(s_sw, 90, 44);
    lv_obj_align(s_sw, LV_ALIGN_TOP_RIGHT, -30, 12);
    if (profilesAutoSwitch()) lv_obj_add_state(s_sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(s_sw, cbSwitch, LV_EVENT_VALUE_CHANGED, nullptr);

    settingsExtrasApplyTheme();
    settingsExtrasTick();
}

static void setIfChanged(lv_obj_t* l, const char* t) { if (strcmp(lv_label_get_text(l), t) != 0) lv_label_set_text(l, t); }

void settingsExtrasTick()
{
    if (!s_box) return;
    const bool on = profilesAutoSwitch();
    const uint32_t seen = s_helperMs;
    const bool helperUp = seen && (uint32_t)(millis() - seen) < HELPER_TIMEOUT_MS;
    setIfChanged(s_state, !on ? "Aus" : helperUp ? "An - Helfer auf dem PC aktiv" : "An - kein Helfer auf dem PC erkannt");
}
