#pragma once
// USB-Geraet zum PC: Composite aus HID-Tastatur (inkl. Consumer/Media-Tasten) und virtueller
// serieller Schnittstelle (CDC). Das ist reine Hardware-Schicht - sie kennt weder Kacheln noch Profile.
#include <Arduino.h>
#include "USB.h"
#include "USBCDC.h"

extern USBCDC DeckSerial;   // die virtuelle COM-Schnittstelle (nicht "Serial": das ist der UART)

// Schaltet GPIO19/20 per IO-Expander auf USB und startet HID + CDC. Nach der Display-/Touch-Init aufrufen.
void usbDeviceInit();

// true, solange der PC das Geraet erkannt hat (und nicht im Standby ist)
bool usbHostConnected();

// Taste(n) gleichzeitig druecken und wieder loslassen. usages = HID-Tastencodes (0x04 = A ...)
// inkl. Modifier (0xE0 = Strg links, 0xE1 = Shift links, 0xE2 = Alt, 0xE3 = Windows ...).
// Blockiert ca. 30 ms - nicht aus dem LVGL-Task aufrufen, sondern ueber deck_actions.
void usbKeyChord(const uint8_t* usages, uint8_t count);

// Eine Consumer-Taste (Lautstaerke, Play/Pause ...) kurz druecken
void usbConsumerTap(uint16_t usage);

// Text tippen (deutsches Tastaturlayout)
void usbTypeText(const char* text);
