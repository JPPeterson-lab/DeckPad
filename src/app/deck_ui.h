#pragma once
// UI-Logik des DeckPad: erzeugt die Kacheln dynamisch aus dem aktiven Profil, verdrahtet Profil-Roller,
// Theme, Helligkeit, Uhr und USB-Status. Kennt LVGL und die Profile, aber kein USB-Protokoll:
// Tastendruecke gehen als deckActionRun() an die Aktionsschicht.
// Alles nur im LVGL-Kontext bzw. mit lvgl_port_lock() aufrufen.

void deckUiInit();                                      // nach ui_init() und nav_init(), Profile muessen geladen sein
void deckUiApplyProfile();                              // Theme, Helligkeit, Titel, Raster des aktiven Profils uebernehmen
void deckUiBootStatus(const char* text, int percent);   // Statuszeile + Balken auf dem Bootscreen
