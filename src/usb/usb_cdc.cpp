#include "usb_cdc.h"
#include "usb_device.h"
#include <string.h>
#include <strings.h>
#include "../app/diag_log.h"

// ── Befehlstabelle ────────────────────────────────────────────────────────────
// Ein Befehl bekommt den Rest der Zeile (ohne Befehlswort) und antwortet ueber usbCdcSendLine().
typedef void (*CmdHandler)(const char* args);
struct Cmd { const char* name; CmdHandler fn; };

static void cmdPing(const char*) { usbCdcSendLine("PONG"); }

static void cmdStatus(const char*) { char b[320]; logStatus(b, sizeof(b)); usbCdcSendLine(b); }
static void emitLine(const char* l) { usbCdcSendLine(l); }
static void cmdLog(const char*) { usbCdcSendLine("--- LOG Anfang ---"); logDump(emitLine); usbCdcSendLine("--- LOG Ende ---"); }

static const Cmd CMDS[] = {
  { "PING",   cmdPing },
  { "STATUS", cmdStatus },   // eine Zeile: Version, Laufzeit, Speicher, Flush-Zeiten
  { "LOG",    cmdLog },      // Ringpuffer der letzten Ereignisse
  // Weitere Befehle (Zeit setzen, Profile uebertragen, aktives Programm melden) folgen in spaeteren Schritten.
};

// ── Zeilenleser ───────────────────────────────────────────────────────────────
#define CDC_LINE_MAX 256     // wird mit den Profil-Befehlen auf mehrere KB wachsen

static void handleLine(char* line)
{
  while (*line == ' ' || *line == '\t') line++;
  size_t n = strlen(line);
  while (n && (line[n - 1] == ' ' || line[n - 1] == '\t')) line[--n] = 0;
  if (!n) return;

  char* args = line;
  while (*args && *args != ' ') args++;
  if (*args) { *args++ = 0; while (*args == ' ') args++; }

  for (const Cmd& c : CMDS) {
    if (strcasecmp(line, c.name) == 0) { c.fn(args); return; }   // Gross-/Kleinschreibung egal
  }
  // Unbekannte Eingaben werden bewusst nicht beantwortet.
}

static void cdcTask(void*)
{
  static char buf[CDC_LINE_MAX];
  size_t len = 0;
  for (;;) {
    while (DeckSerial.available()) {
      int c = DeckSerial.read();
      if (c == '\n' || c == '\r') {
        if (len) { buf[len] = 0; handleLine(buf); len = 0; }
      } else if (len < CDC_LINE_MAX - 1) {
        buf[len++] = (char)c;
      } else {
        len = 0;                    // Zeile zu lang: verwerfen
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void usbCdcSendLine(const char* line)
{
  if (!DeckSerial) return;          // kein Terminal geoeffnet (DTR aus): nichts senden, nicht blockieren
  DeckSerial.print(line);
  DeckSerial.print("\r\n");
}

void usbCdcInit()
{
  xTaskCreatePinnedToCore(cdcTask, "cdc", 4096, nullptr, 1, nullptr, 0);   // Core 0, LVGL laeuft auf Core 1
}
