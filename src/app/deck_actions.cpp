#include "deck_actions.h"
#include <Arduino.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "../usb/usb_device.h"
#include "../usb/usb_cdc.h"
#include "USBHIDConsumerControl.h"

// ── Namenstabellen ────────────────────────────────────────────────────────────
struct NameCode { const char* name; uint16_t code; };

static const NameCode MEDIA[] = {
  { "vol_up",     CONSUMER_CONTROL_VOLUME_INCREMENT },
  { "vol_down",   CONSUMER_CONTROL_VOLUME_DECREMENT },
  { "mute",       CONSUMER_CONTROL_MUTE },
  { "play_pause", CONSUMER_CONTROL_PLAY_PAUSE },
  { "next",       CONSUMER_CONTROL_SCAN_NEXT },
  { "prev",       CONSUMER_CONTROL_SCAN_PREVIOUS },
  { "stop",       CONSUMER_CONTROL_STOP },
  { "calculator", CONSUMER_CONTROL_CALCULATOR },
  { "browser",    CONSUMER_CONTROL_LOCAL_BROWSER },
  { "mail",       CONSUMER_CONTROL_EMAIL_READER },
};

// HID-Usage-Codes (Tastaturseite). Modifier liegen bei 0xE0..0xE7.
static const NameCode KEYS[] = {
  { "ctrl", 0xE0 }, { "shift", 0xE1 }, { "alt", 0xE2 }, { "win", 0xE3 }, { "gui", 0xE3 },
  { "rctrl", 0xE4 }, { "rshift", 0xE5 }, { "altgr", 0xE6 }, { "rwin", 0xE7 },
  { "enter", 0x28 }, { "esc", 0x29 }, { "backspace", 0x2A }, { "tab", 0x2B }, { "space", 0x2C },
  { "printscreen", 0x46 }, { "scrolllock", 0x47 }, { "pause", 0x48 }, { "insert", 0x49 },
  { "home", 0x4A }, { "pageup", 0x4B }, { "delete", 0x4C }, { "end", 0x4D }, { "pagedown", 0x4E },
  { "right", 0x4F }, { "left", 0x50 }, { "down", 0x51 }, { "up", 0x52 }, { "capslock", 0x39 },
};

static int lookup(const NameCode* t, size_t n, const char* s, size_t len)
{
  for (size_t i = 0; i < n; i++)
    if (strlen(t[i].name) == len && strncasecmp(t[i].name, s, len) == 0) return (int)t[i].code;
  return -1;
}

// Ein Tastenname -> HID-Code: Namenstabelle, a-z, 0-9, f1-f24
static int keyCode(const char* s, size_t len)
{
  int c = lookup(KEYS, sizeof(KEYS) / sizeof(KEYS[0]), s, len);
  if (c >= 0) return c;
  if (len == 1) {
    char ch = (char)tolower((unsigned char)s[0]);
    if (ch >= 'a' && ch <= 'z') return 0x04 + (ch - 'a');
    if (ch >= '1' && ch <= '9') return 0x1E + (ch - '1');
    if (ch == '0') return 0x27;
  }
  if ((s[0] == 'f' || s[0] == 'F') && len >= 2 && len <= 3) {
    int n = atoi(s + 1);
    if (n >= 1 && n <= 12) return 0x3A + (n - 1);
    if (n >= 13 && n <= 24) return 0x68 + (n - 13);
  }
  return -1;
}

// "ctrl+shift+m" -> {0xE0, 0xE1, 0x10}; Rueckgabe: Anzahl oder 0 bei Syntaxfehler (max. 8 Teile)
static uint8_t parseKeys(const char* spec, uint8_t* out, uint8_t maxN)
{
  uint8_t n = 0;
  const char* p = spec;
  while (*p) {
    const char* e = p;
    while (*e && *e != '+') e++;
    size_t len = (size_t)(e - p);
    if (!len || n >= maxN) return 0;
    int c = keyCode(p, len);
    if (c < 0) return 0;
    out[n++] = (uint8_t)c;
    p = *e ? e + 1 : e;
  }
  return n;
}

// ── Namen <-> Typ ─────────────────────────────────────────────────────────────
static const char* const TYPE_NAMES[] = { "none", "media", "keys", "text", "host" };

const char* deckActionTypeName(DeckActionType t) { return t <= ACT_HOST ? TYPE_NAMES[t] : "none"; }

DeckActionType deckActionTypeFromName(const char* name)
{
  if (name) for (uint8_t i = 0; i <= ACT_HOST; i++) if (strcasecmp(name, TYPE_NAMES[i]) == 0) return (DeckActionType)i;
  return ACT_NONE;
}

bool deckActionValid(DeckActionType type, const char* spec)
{
  uint8_t tmp[8];
  switch (type) {
    case ACT_NONE:  return true;
    case ACT_MEDIA: return lookup(MEDIA, sizeof(MEDIA) / sizeof(MEDIA[0]), spec, strlen(spec)) >= 0;
    case ACT_KEYS:  return parseKeys(spec, tmp, sizeof(tmp)) > 0;
    case ACT_TEXT:
    case ACT_HOST:  return spec && *spec;
  }
  return false;
}

// ── Task: fuehrt die Aktionen nacheinander aus ───────────────────────────────
struct Job { DeckActionType type; char spec[DECK_SPEC_MAX]; };
static QueueHandle_t s_queue = nullptr;

static void execute(const Job& j)
{
  // Auch ohne gemeldete Verbindung senden: die USB-Bibliothek verwirft Berichte, wenn der PC nicht bereit ist,
  // und ein verpasstes Verbindungs-Ereignis soll nicht alle Tasten stilllegen.
  if (!usbHostConnected() && j.type != ACT_NONE)
    Serial.printf("[Aktion] %s '%s': PC meldet sich nicht als verbunden\n", deckActionTypeName(j.type), j.spec);
  switch (j.type) {
    case ACT_MEDIA: {
      int c = lookup(MEDIA, sizeof(MEDIA) / sizeof(MEDIA[0]), j.spec, strlen(j.spec));
      if (c >= 0) usbConsumerTap((uint16_t)c);
      break;
    }
    case ACT_KEYS: {
      uint8_t k[8];
      uint8_t n = parseKeys(j.spec, k, sizeof(k));
      if (n) usbKeyChord(k, n);
      break;
    }
    case ACT_TEXT: usbTypeText(j.spec); break;
    case ACT_HOST: {
      char line[DECK_SPEC_MAX + 12];
      snprintf(line, sizeof(line), "ACTION %s", j.spec);
      usbCdcSendLine(line);
      break;
    }
    default: break;
  }
}

static void actionTask(void*)
{
  Job j;
  for (;;) if (xQueueReceive(s_queue, &j, portMAX_DELAY) == pdTRUE) execute(j);
}

void deckActionsInit()
{
  s_queue = xQueueCreate(8, sizeof(Job));
  xTaskCreatePinnedToCore(actionTask, "actions", 4096, nullptr, 2, nullptr, 0);
}

bool deckActionRun(DeckActionType type, const char* spec)
{
  if (type == ACT_NONE || !s_queue) return true;
  Job j;
  j.type = type;
  strlcpy(j.spec, spec ? spec : "", sizeof(j.spec));
  return xQueueSend(s_queue, &j, 0) == pdTRUE;
}
