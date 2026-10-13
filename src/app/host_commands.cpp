#include "host_commands.h"
#include <Arduino.h>
#include <sys/time.h>
#include "app_config.h"
#include "profiles.h"
#include "deck_ui.h"
#include "diag_log.h"
#include "lv_lock.h"
#include "../usb/usb_cdc.h"

// Antworten: Einzeiler. Erfolg "OK", Fehler "ERR <Grund>", LIST/GET/STATUS liefern direkt ihre Daten.
// Wichtig: Datei-Zugriffe (profiles.*) laufen hier im CDC-Task, alles an LVGL erst danach unter LvLock.
// Nie umgekehrt (erst LvLock, dann Profil-Mutex) - sonst Deadlock mit dem LVGL-Task.

static void err(const char* why) { char b[96]; snprintf(b, sizeof(b), "ERR %s", why); usbCdcSendLine(b); }

static void cmdVer(const char*)  { usbCdcSendLine("DECKPAD " FIRMWARE_VERSION); }

static void cmdList(const char*) { String s; profileListJson(s); usbCdcSendLine(s.c_str()); }

static void cmdGet(const char* a)
{
  String s;
  if (!profileReadRaw(a, s)) { err("unbekannt"); return; }
  usbCdcSendLine(s.c_str());
}

// PUT <id> <json>   (legt an oder ueberschreibt)
static void cmdPut(const char* a)
{
  char id[32];
  const char* sp = strchr(a, ' ');
  if (!sp || (size_t)(sp - a) >= sizeof(id)) { err("Format: PUT <id> <json>"); return; }
  memcpy(id, a, sp - a); id[sp - a] = 0;
  const char* why = "";
  if (!profileImportJson(id, sp + 1, why)) { err(why); return; }
  { LvLock lock; deckUiProfilesChanged(); }
  logDiag("[PC] Profil '%s' empfangen", id);
  usbCdcSendLine("OK");
}

static void cmdDel(const char* a)
{
  if (!profileDelete(a)) { err("nicht geloescht (unbekannt oder letztes Profil)"); return; }
  { LvLock lock; deckUiProfilesChanged(); }
  logDiag("[PC] Profil '%s' geloescht", a);
  usbCdcSendLine("OK");
}

static void cmdUse(const char* a)
{
  bool ok;
  { LvLock lock; ok = deckUiSelectProfile(a); }
  if (ok) usbCdcSendLine("OK"); else err("unbekannt");
}

// TIME <Unix-Sekunden UTC>   (die Zeitzone CET/CEST ist im Geraet hinterlegt)
static void cmdTime(const char* a)
{
  long long t = atoll(a);
  if (t < 1700000000LL) { err("Zeit ungueltig"); return; }
  struct timeval tv = { (time_t)t, 0 };
  settimeofday(&tv, nullptr);
  logDiag("[PC] Uhrzeit gesetzt");
  usbCdcSendLine("OK");
}

static void cmdStatus(const char*) { char b[320]; logStatus(b, sizeof(b)); usbCdcSendLine(b); }
static void emitLine(const char* l) { usbCdcSendLine(l); }
static void cmdLog(const char*) { usbCdcSendLine("--- LOG Anfang ---"); logDump(emitLine); usbCdcSendLine("--- LOG Ende ---"); }

void hostCommandsInit()
{
  usbCdcRegister("VER",    cmdVer);
  usbCdcRegister("LIST",   cmdList);
  usbCdcRegister("GET",    cmdGet);
  usbCdcRegister("PUT",    cmdPut);
  usbCdcRegister("DEL",    cmdDel);
  usbCdcRegister("USE",    cmdUse);
  usbCdcRegister("TIME",   cmdTime);
  usbCdcRegister("STATUS", cmdStatus);
  usbCdcRegister("LOG",    cmdLog);
}
