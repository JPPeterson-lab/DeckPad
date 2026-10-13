#pragma once
// Kommandokanal ueber die virtuelle COM-Schnittstelle. Zeilenbasiert (Abschluss \n oder \r\n).
// Aktueller Stand: nur "PING" -> "PONG". Neue Befehle werden in usb_cdc.cpp in die Tabelle eingetragen.
void usbCdcInit();                         // Lese-Task starten (nach usbDeviceInit)
void usbCdcSendLine(const char* line);     // eine Zeile an den PC schicken (z. B. fuer Host-Aktionen)
