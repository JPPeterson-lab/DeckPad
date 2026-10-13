#pragma once
// Zusatz auf dem Einstellungs-Screen, im Code angelegt: Schalter "Automatischer Profilwechsel" samt Helfer-Status.
// (Kann spaeter nach PicoPixel wandern; dann die Objekte dort anlegen und diese Datei durch Verdrahtung ersetzen.)
// Nur im LVGL-Kontext aufrufen, ausser settingsExtrasHelperSeen().
void settingsExtrasInit();         // nach ui_init() und profilesInit()
void settingsExtrasApplyTheme();   // Farben an das aktive Theme anpassen
void settingsExtrasTick();         // jede Sekunde: Statustext
void settingsExtrasHelperSeen();   // vom CDC-Task: der PC-Helfer hat sich gemeldet (thread-sicher)
