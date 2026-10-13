#!/bin/bash
# Baut den ESP-IDF-Versuchsbuild (ESP-IDF 5.5.5 muss unter ~/esp/esp-idf-v5.5.5 installiert sein).
#   bash build.sh              nur bauen
#   bash build.sh flash PORT   bauen und auf den UART-Port des Boards flashen (z. B. /dev/cu.usbserial-1234)
#   bash build.sh clean        build/ und sdkconfig loeschen (z. B. nach Aenderungen an sdkconfig.defaults)
source ~/esp/esp-idf-v5.5.5/export.sh > /dev/null 2>&1 || { echo "ESP-IDF 5.5.5 nicht gefunden"; exit 1; }
cd "$(dirname "$0")" || exit 1
case "$1" in
  clean) rm -rf build sdkconfig sdkconfig.old; echo "aufgeraeumt"; exit 0 ;;
esac
[ -f sdkconfig ] || idf.py set-target esp32s3
# Pixeltakt in MHz per Umgebungsvariable, z. B.  PCLK=40 bash build.sh flash /dev/cu.xxx  (Standard 30)
DEFS=""; [ -n "$PCLK" ] && DEFS="-DDECK_PIXEL_CLOCK_MHZ=$PCLK"
if [ "$1" = "flash" ]; then
  [ -n "$2" ] || { echo "Verwendung: bash build.sh flash <PORT>"; exit 1; }
  idf.py $DEFS -p "$2" flash
else
  idf.py $DEFS build
fi
