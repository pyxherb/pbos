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

target_include_directories(bootldx SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR})
add_dependencies(bootldx oicarch freestdc)
target_link_libraries(bootldx oicarch freestdc)
target_link_options(bootldx PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/bootldr.lds")

add_custom_target(bootldr
    COMMAND ${CMAKE_OBJCOPY} --strip-debug -O binary $<TARGET_FILE:bootldx> ${OICOS_BINARY_DIR}/bootldr
    BYPRODUCTS ${OICOS_BINARY_DIR}/bootldr
    DEPENDS bootldx)
