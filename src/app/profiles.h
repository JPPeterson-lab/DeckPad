#pragma once
// Profile: pro Programm eigene Tasten (Kacheln) und eigene Einstellungen (Theme, Helligkeit, Raster).
// Jedes Profil ist eine JSON-Datei in /profiles/<id>.json auf der Partition "storage" (LittleFS).
// Diese Schicht kennt weder LVGL noch USB - nur Daten und Dateien.
#include <Arduino.h>
#include "app_config.h"
#include "deck_actions.h"

struct DeckTile {
  char           label[28];          // Beschriftung, '\n' fuer eine zweite Zeile
  uint32_t       color;              // 0xRRGGBB
  DeckActionType type;
  char           spec[DECK_SPEC_MAX];
  char           icon[24];           // Icon-ID (Datei /icons/<id>.bin), leer = kein Icon
};

struct DeckProfile {
  char     id[24];                   // Dateiname ohne ".json" (a-z, 0-9, '-', '_')
  char     name[32];                 // Anzeigename
  char     exe[48];                  // Programmname fuer den spaeteren Auto-Wechsel (z. B. "bambu-studio.exe")
  uint8_t  cols, rows;
  uint8_t  theme;                    // 0 = hell, 1 = dunkel (ThemesEnum aus PicoPixel)
  uint8_t  brightness;               // 3..100 %
  uint8_t  tileCount;                // belegte Kacheln, zeilenweise; Rest des Rasters bleibt leer
  DeckTile tiles[DECK_MAX_TILES];
};

struct ProfileInfo { char id[24]; char name[32]; char exe[48]; };

bool  profilesInit();                              // Dateisystem mounten, Beispielprofile anlegen, aktives laden
bool  profilesStorageOk();                         // false: Flash-Speicher nicht nutzbar (RAM-Notprofil aktiv)
int   profileCount();
const ProfileInfo& profileInfo(int index);
int   profileActiveIndex();
const DeckProfile& profileActive();
bool  profileActivate(int index);                  // laden; die Auswahl wird verzoegert gespeichert (siehe profilesTick)
bool  profilesAutoSwitch();                        // automatischer Profilwechsel nach dem Vordergrund-Programm (Standard: aus)
void  profilesSetAutoSwitch(bool on);              // wird verzoegert gespeichert
int   profileIndexByExe(const char* exe);          // Profil zu einem Programmnamen (ohne Beachtung von Gross-/Kleinschreibung und ".exe"), -1 = keins
void  profilesTick();                              // regelmaessig aufrufen (1 s): schreibt die Auswahl, wenn 8 s nichts gewechselt wurde
void  profileNamesJoined(String& out);             // "A\nB\nC" fuer das Roller-Widget

// Settings des aktiven Profils schreiben (Theme/Helligkeit): nur diese beiden Werte, Tasten bleiben unberuehrt
bool  profileSaveActiveSettings(uint8_t theme, uint8_t brightness);

// Fuer den spaeteren Import ueber CDC/PC-Programm
bool  profileLoad(const char* id, DeckProfile& out);
bool  profileSave(const DeckProfile& p);
bool  profileDelete(const char* id);

// Fuer die Befehle vom PC (CDC). Nur Datei-Zugriffe - das aktive Profil im RAM aendert ausschliesslich der LVGL-Task.
void  profilesRescan();                                   // Dateiliste neu lesen; aktives Profil bleibt (per id) erhalten
int   profileIndexOf(const char* id);                     // -1 = unbekannt
void  profileListJson(String& out);                       // {"active":"id","profiles":[{"id","name","exe"},...]}
bool  profileReadRaw(const char* id, String& out);        // Dateiinhalt (eine Zeile JSON)
bool  profileImportJson(const char* id, const char* json, const char*& err);   // pruefen und speichern
