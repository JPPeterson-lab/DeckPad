# Alle Quellen liegen im Repo-Stamm (src/), nicht hier: so gibt es nur eine Kopie des Codes.
get_filename_component(DECK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(GLOB_RECURSE DECK_SRCS "${DECK_ROOT}/src/*.c" "${DECK_ROOT}/src/*.cpp")

idf_component_register(
    SRCS "main.cpp" ${DECK_SRCS}
    INCLUDE_DIRS "." "${DECK_ROOT}" "${DECK_ROOT}/src"
)
# Wie in der Arduino-IDE: Compiler-Schalter aus build_opt.h (LVGL-Pool im PSRAM usw.) werden in sdkconfig/lv_conf.h abgebildet.
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error)
# Versuchsbuild: kein USB-Geraet (siehe usb_device.h). Befehle laufen ueber den UART-Port.
option(DECK_NO_USB "ohne USB-Geraet bauen (Befehle ueber den UART-Port)" OFF)
if(DECK_NO_USB)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE DECK_NO_USB=1)
else()
    target_compile_definitions(${COMPONENT_LIB} PRIVATE DECK_USB_IDF=1)
endif()
# Pixeltakt in MHz (Bildrate = Takt / (1386 x 661)): idf.py -DDECK_PIXEL_CLOCK_MHZ=40 build
if(NOT DEFINED DECK_PIXEL_CLOCK_MHZ)
    set(DECK_PIXEL_CLOCK_MHZ 30)
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE DECK_PIXEL_CLOCK_HZ=${DECK_PIXEL_CLOCK_MHZ}000000)

# Die Arduino-Komponente uebersetzt ihre eigenen USB-Klassen (USBCDC, USBHID ...), sobald TinyUSB aktiv ist. Sie passen nicht zur
# IDF-TinyUSB-Schicht, die wir selbst stellen (src/usb/usb_device_idf.cpp): diese Quellen aus dem Build nehmen.
if(NOT DECK_NO_USB AND TARGET __idf_espressif__arduino-esp32)
    get_target_property(_arduino_srcs __idf_espressif__arduino-esp32 SOURCES)
    list(FILTER _arduino_srcs EXCLUDE REGEX "cores/esp32/(USB|USBCDC|USBMSC|USBHost|FirmwareMSC|firmware_msc_fat|esp32-hal-tinyusb)\\.(c|cpp)$")
    set_property(TARGET __idf_espressif__arduino-esp32 PROPERTY SOURCES "${_arduino_srcs}")
endif()
