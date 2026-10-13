#include "usb_device.h"
#include "USBHIDKeyboard.h"
#include "USBHIDConsumerControl.h"
#include "../hardware/io_extension.h"
#include "../app/app_config.h"

USBCDC DeckSerial;
static USBHIDKeyboard        s_keyboard;
static USBHIDConsumerControl s_consumer;
static volatile bool         s_hostUp = false;

static void onUsbEvent(void*, esp_event_base_t, int32_t id, void*)
{
  switch (id) {
    case ARDUINO_USB_STARTED_EVENT: s_hostUp = true;  break;   // PC hat das Geraet erkannt
    case ARDUINO_USB_RESUME_EVENT:  s_hostUp = true;  break;
    case ARDUINO_USB_STOPPED_EVENT: s_hostUp = false; break;   // Kabel ab
    case ARDUINO_USB_SUSPEND_EVENT: s_hostUp = false; break;   // PC im Standby
    default: break;
  }
}

void usbDeviceInit()
{
  // Die nativen USB-Pins GPIO19/20 teilt sich das Board mit dem CAN-Transceiver. Der IO-Expander
  // (IO5) entscheidet, wer sie bekommt. Der Expander merkt sich alle Ausgaenge: einmal gesetzt,
  // bleibt IO5 auch bei den Schreibzugriffen auf Backlight/Touch-Reset unveraendert.
  IO_EXTENSION_Output(IO_EXTENSION_IO_5, DECK_USB_SELECT_LEVEL);
  Serial.printf("[USB] IO5 = %d (USB-Pfad aktiv)\n", DECK_USB_SELECT_LEVEL);

  USB.onEvent(onUsbEvent);
  USB.manufacturerName(DECK_USB_MANUFACTURER);
  USB.productName(DECK_USB_PRODUCT);

  s_keyboard.begin(KeyboardLayout_de_DE);   // nur fuer usbTypeText; Tastenkombis nutzen Raw-Codes
  s_consumer.begin();
  DeckSerial.setRxBufferSize(8192);         // Platz fuer spaeter uebertragene Profile (JSON)
  DeckSerial.begin();
  USB.begin();
  Serial.println("[USB] HID-Tastatur + Consumer + CDC gestartet");
}

bool usbHostConnected() { return s_hostUp; }

void usbKeyChord(const uint8_t* usages, uint8_t count)
{
  for (uint8_t i = 0; i < count; i++) s_keyboard.pressRaw(usages[i]);
  delay(25);                                // kurz halten, damit Windows die Kombination sicher erkennt
  s_keyboard.releaseAll();
}

void usbConsumerTap(uint16_t usage)
{
  s_consumer.press(usage);
  delay(25);
  s_consumer.release();
}

void usbTypeText(const char* text)
{
  if (text && *text) s_keyboard.print(text);
}
