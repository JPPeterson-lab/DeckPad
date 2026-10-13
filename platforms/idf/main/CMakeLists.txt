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
target_compile_definitions(${COMPONENT_LIB} PRIVATE DECK_NO_USB=1)
