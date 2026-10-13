#include "profiles.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <string.h>

static ProfileInfo  s_list[DECK_MAX_PROFILES];
static int          s_count   = 0;
static int          s_active  = 0;
static DeckProfile  s_cur;                 // geladenes aktives Profil (liegt im internen RAM, ca. 3 KB)
static bool         s_fsOk    = false;
static SemaphoreHandle_t s_lock = nullptr;

struct Guard { Guard() { xSemaphoreTakeRecursive(s_lock, portMAX_DELAY); } ~Guard() { xSemaphoreGiveRecursive(s_lock); } };

// ── Hilfen ────────────────────────────────────────────────────────────────────
static String pathOf(const char* id) { return String(DECK_PROFILE_DIR) + "/" + id + ".json"; }

static bool idValid(const char* id)
{
  size_t n = strlen(id);
  if (!n || n >= sizeof(((DeckProfile*)0)->id)) return false;
  for (size_t i = 0; i < n; i++) {
    char c = id[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) return false;   // verhindert "../"
  }
  return true;
}

static uint32_t parseColor(const char* s, uint32_t def)
{
  if (!s) return def;
  if (*s == '#') s++;
  char* end = nullptr;
  unsigned long v = strtoul(s, &end, 16);
  return (end && *end == 0 && end != s) ? (uint32_t)(v & 0xFFFFFF) : def;
}

static void defaultsInto(DeckProfile& p)
{
  memset(&p, 0, sizeof(p));
  p.cols = DECK_DEFAULT_COLS;
  p.rows = DECK_DEFAULT_ROWS;
  p.brightness = DECK_BRIGHTNESS_DEFAULT;
}

// ── JSON <-> Profil ───────────────────────────────────────────────────────────
static bool fromJson(const char* id, JsonDocument& doc, DeckProfile& p)
{
  defaultsInto(p);
  strlcpy(p.id, id, sizeof(p.id));
  strlcpy(p.name, doc["name"] | id, sizeof(p.name));
  strlcpy(p.exe,  doc["exe"]  | "", sizeof(p.exe));
  p.cols = constrain((int)(doc["cols"] | DECK_DEFAULT_COLS), 1, DECK_MAX_COLS);
  p.rows = constrain((int)(doc["rows"] | DECK_DEFAULT_ROWS), 1, DECK_MAX_ROWS);
  JsonObject st = doc["settings"];
  p.theme      = strcmp(st["theme"] | "light", "dark") == 0 ? 1 : 0;
  p.brightness = constrain((int)(st["brightness"] | DECK_BRIGHTNESS_DEFAULT), DECK_BRIGHTNESS_MIN, 100);

  uint8_t maxTiles = p.cols * p.rows;
  for (JsonObject t : doc["tiles"].as<JsonArray>()) {
    if (p.tileCount >= maxTiles) break;
    DeckTile& d = p.tiles[p.tileCount++];
    strlcpy(d.label, t["label"] | "", sizeof(d.label));
    d.color = parseColor(t["color"] | "", 0x34495e);
    d.type  = deckActionTypeFromName(t["type"] | "none");
    strlcpy(d.spec, t["spec"] | "", sizeof(d.spec));
    if (!deckActionValid(d.type, d.spec)) { d.type = ACT_NONE; d.spec[0] = 0; }   // kaputte Aktion nie ausfuehren
  }
  return true;
}

static void toJson(const DeckProfile& p, JsonDocument& doc)
{
  doc["v"] = 1;
  doc["name"] = p.name;
  doc["exe"]  = p.exe;
  doc["cols"] = p.cols;
  doc["rows"] = p.rows;
  JsonObject st = doc["settings"].to<JsonObject>();
  st["theme"] = p.theme ? "dark" : "light";
  st["brightness"] = p.brightness;
  JsonArray a = doc["tiles"].to<JsonArray>();
  for (uint8_t i = 0; i < p.tileCount; i++) {
    const DeckTile& t = p.tiles[i];
    JsonObject o = a.add<JsonObject>();
    o["label"] = t.label;
    char col[8]; snprintf(col, sizeof(col), "#%06X", (unsigned)t.color);
    o["color"] = col;
    o["type"]  = deckActionTypeName(t.type);
    o["spec"]  = t.spec;
  }
}

// ── Dateien ───────────────────────────────────────────────────────────────────
bool profileLoad(const char* id, DeckProfile& out)
{
  Guard g;
  if (!s_fsOk || !idValid(id)) return false;
  File f = LittleFS.open(pathOf(id), "r");
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) { Serial.printf("[Profile] %s: JSON-Fehler %s\n", id, err.c_str()); return false; }
  return fromJson(id, doc, out);
}

bool profileSave(const DeckProfile& p)
{
  Guard g;
  if (!s_fsOk || !idValid(p.id)) return false;
  JsonDocument doc;
  toJson(p, doc);
  String path = pathOf(p.id);
  String tmp  = path + ".tmp";                  // erst in Temp-Datei, dann umbenennen: kein halbes Profil bei Stromausfall
  File f = LittleFS.open(tmp, "w");
  if (!f) return false;
  size_t w = serializeJson(doc, f);
  f.close();
  if (!w) { LittleFS.remove(tmp); return false; }
  LittleFS.remove(path);
  return LittleFS.rename(tmp, path);
}

bool profileDelete(const char* id)
{
  Guard g;
  if (!s_fsOk || !idValid(id) || s_count <= 1) return false;     // das letzte Profil bleibt immer erhalten
  return LittleFS.remove(pathOf(id));
}

static void scanList()
{
  s_count = 0;
  File dir = LittleFS.open(DECK_PROFILE_DIR);
  if (!dir || !dir.isDirectory()) return;
  for (File f = dir.openNextFile(); f && s_count < DECK_MAX_PROFILES; f = dir.openNextFile()) {
    String n = f.name();                          // nur der Dateiname, z. B. "standard.json"
    int slash = n.lastIndexOf('/'); if (slash >= 0) n = n.substring(slash + 1);
    if (f.isDirectory() || !n.endsWith(".json")) continue;
    n.remove(n.length() - 5);
    if (!idValid(n.c_str())) continue;
    JsonDocument doc;
    if (deserializeJson(doc, f)) continue;        // defekte Datei ueberspringen
    ProfileInfo& pi = s_list[s_count++];
    strlcpy(pi.id, n.c_str(), sizeof(pi.id));
    strlcpy(pi.name, doc["name"] | pi.id, sizeof(pi.name));
    strlcpy(pi.exe, doc["exe"] | "", sizeof(pi.exe));
  }
  // alphabetisch nach Anzeigename, "Standard" soll aber stabil vorne bleiben -> nach id sortieren
  for (int i = 1; i < s_count; i++)
    for (int j = i; j > 0 && strcmp(s_list[j - 1].id, s_list[j].id) > 0; j--) { ProfileInfo t = s_list[j]; s_list[j] = s_list[j - 1]; s_list[j - 1] = t; }
}

// ── Beispielprofile beim ersten Start ─────────────────────────────────────────
static void addTile(DeckProfile& p, const char* label, uint32_t color, DeckActionType t, const char* spec)
{
  DeckTile& d = p.tiles[p.tileCount++];
  strlcpy(d.label, label, sizeof(d.label)); d.color = color; d.type = t; strlcpy(d.spec, spec, sizeof(d.spec));
}

static void seedProfiles()
{
  static const uint32_t C[] = { 0x2980b9, 0x16a085, 0x8e44ad, 0xd35400, 0xc0392b, 0x2c3e50 };
  DeckProfile p;

  defaultsInto(p);
  strlcpy(p.id, "standard", sizeof(p.id)); strlcpy(p.name, "Standard", sizeof(p.name));
  addTile(p, "Lauter",      0x27ae60, ACT_MEDIA, "vol_up");          // Testtaste 1
  addTile(p, "Leiser",      0x2980b9, ACT_MEDIA, "vol_down");        // Testtaste 2
  addTile(p, "Strg+Shift+M", 0x8e44ad, ACT_KEYS, "ctrl+shift+m");    // Testtaste 3
  addTile(p, "Stumm",       0xc0392b, ACT_MEDIA, "mute");
  addTile(p, "Play\nPause", 0xd35400, ACT_MEDIA, "play_pause");
  addTile(p, "Zurück",     0x16a085, ACT_MEDIA, "prev");
  addTile(p, "Weiter",      0x16a085, ACT_MEDIA, "next");
  for (int i = p.tileCount; i < p.cols * p.rows; i++) {            // Rest: Test-Raster ohne Funktion
    char l[12]; snprintf(l, sizeof(l), "Taste %d", i + 1);
    addTile(p, l, C[i % 6], ACT_NONE, "");
  }
  profileSave(p);

  defaultsInto(p);                                                 // Beispiel fuer ein Programm-Profil mit eigenem Theme
  strlcpy(p.id, "bambu", sizeof(p.id)); strlcpy(p.name, "Bambu Studio", sizeof(p.name));
  strlcpy(p.exe, "bambu-studio.exe", sizeof(p.exe));
  p.theme = 1; p.brightness = 70;
  addTile(p, "Rückgängig", 0x2980b9, ACT_KEYS, "ctrl+z");
  addTile(p, "Wiederholen",  0x2980b9, ACT_KEYS, "ctrl+y");
  addTile(p, "Speichern",    0x27ae60, ACT_KEYS, "ctrl+s");
  addTile(p, "Alles\nwählen", 0x8e44ad, ACT_KEYS, "ctrl+a");
  addTile(p, "Ansicht\nanpassen", 0xd35400, ACT_KEYS, "ctrl+0");
  profileSave(p);
}

// ── Aktives Profil ────────────────────────────────────────────────────────────
static void builtinFallback()                      // wenn der Flash-Speicher nicht geht: wenigstens eine Taste
{
  defaultsInto(s_cur);
  strlcpy(s_cur.id, "notfall", sizeof(s_cur.id)); strlcpy(s_cur.name, "Notfall (RAM)", sizeof(s_cur.name));
  addTile(s_cur, "Lauter", 0x27ae60, ACT_MEDIA, "vol_up");
  addTile(s_cur, "Leiser", 0x2980b9, ACT_MEDIA, "vol_down");
  addTile(s_cur, "Strg+Shift+M", 0x8e44ad, ACT_KEYS, "ctrl+shift+m");
  s_count = 1;
  strlcpy(s_list[0].id, s_cur.id, sizeof(s_list[0].id)); strlcpy(s_list[0].name, s_cur.name, sizeof(s_list[0].name));
  s_active = 0;
}

static void saveConfig()
{
  if (!s_fsOk || s_count == 0) return;
  JsonDocument doc;
  doc["active"] = s_list[s_active].id;
  File f = LittleFS.open(DECK_CONFIG_FILE, "w");
  if (f) { serializeJson(doc, f); f.close(); }
}

bool profileActivate(int index)
{
  Guard g;
  if (index < 0 || index >= s_count) return false;
  if (s_fsOk) {
    if (!profileLoad(s_list[index].id, s_cur)) return false;
  }
  s_active = index;
  saveConfig();
  return true;
}

bool profilesInit()
{
  s_lock = xSemaphoreCreateRecursiveMutex();
  Guard g;
  s_fsOk = LittleFS.begin(true, "/littlefs", 10, DECK_STORAGE_LABEL);   // true = beim ersten Mal formatieren
  if (!s_fsOk) {
    Serial.println("[Profile] LittleFS-Partition 'storage' nicht verfuegbar - Notprofil im RAM");
    builtinFallback();
    return false;
  }
  Serial.printf("[Profile] Speicher: %u von %u KB belegt\n", (unsigned)(LittleFS.usedBytes() / 1024), (unsigned)(LittleFS.totalBytes() / 1024));
  if (!LittleFS.exists(DECK_PROFILE_DIR)) LittleFS.mkdir(DECK_PROFILE_DIR);
  scanList();
  if (s_count == 0) { Serial.println("[Profile] keine Profile gefunden - lege Beispielprofile an"); seedProfiles(); scanList(); }
  if (s_count == 0) { builtinFallback(); s_fsOk = false; return false; }

  // zuletzt aktives Profil wiederherstellen; ohne Config-Datei startet "Standard"
  s_active = 0;
  for (int i = 0; i < s_count; i++) if (strcmp(s_list[i].id, "standard") == 0) s_active = i;
  File f = LittleFS.open(DECK_CONFIG_FILE, "r");
  if (f) {
    JsonDocument doc;
    if (!deserializeJson(doc, f)) {
      const char* a = doc["active"] | "";
      for (int i = 0; i < s_count; i++) if (strcmp(s_list[i].id, a) == 0) s_active = i;
    }
    f.close();
  }
  if (!profileLoad(s_list[s_active].id, s_cur)) {                 // defekt: das naechste lesbare Profil nehmen
    bool ok = false;
    for (int i = 0; i < s_count && !ok; i++) if (profileLoad(s_list[i].id, s_cur)) { s_active = i; ok = true; }
    if (!ok) { builtinFallback(); return false; }
  }
  Serial.printf("[Profile] %d Profile, aktiv: %s\n", s_count, s_cur.name);
  return true;
}

bool profileSaveActiveSettings(uint8_t theme, uint8_t brightness)
{
  Guard g;
  s_cur.theme = theme;
  s_cur.brightness = constrain((int)brightness, DECK_BRIGHTNESS_MIN, 100);
  return s_fsOk ? profileSave(s_cur) : false;
}

bool profilesStorageOk() { return s_fsOk; }
int  profileCount() { return s_count; }
const ProfileInfo& profileInfo(int i) { return s_list[constrain(i, 0, max(s_count - 1, 0))]; }
int  profileActiveIndex() { return s_active; }
const DeckProfile& profileActive() { return s_cur; }

void profileNamesJoined(String& out)
{
  out = "";
  for (int i = 0; i < s_count; i++) { if (i) out += "\n"; out += s_list[i].name; }
}
