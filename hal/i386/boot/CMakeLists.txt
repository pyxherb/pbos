project(bootldr)

add_executable(bootldx
    "init.S"
    "main.c"
    "error.h" "error.c"
    "kloader.h" "kloader.c"
    "mb.h" "mb.c"
    "misc.h"
    "../display/vga.c")

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)

target_include_directories(bootldx SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR} ${FREESTDC_INCLUDE_DIR_CXX})
add_dependencies(bootldx pbarch freestdc)
target_link_libraries(bootldx pbarch freestdc)
target_link_options(bootldx PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/bootldr.lds")

add_custom_target(bootldr
    COMMAND ${CMAKE_OBJCOPY} --strip-debug -O binary $<TARGET_FILE:bootldx> ${PBOS_BINARY_DIR}/bootldr
    BYPRODUCTS ${PBOS_BINARY_DIR}/bootldr
    DEPENDS bootldx)
