find_package(sctgen REQUIRED)

file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbkrnl PRIVATE ${SRC} "${PROJECT_SOURCE_DIR}/include/pbos/generated/syscall/pbcore.h")

add_custom_command(
    OUTPUT
        "${PROJECT_SOURCE_DIR}/include/pbos/generated/syscall/pbcore.h"
        "${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/generated/pbcore.cc"
    COMMAND
        ${SCTGEN_EXECUTABLE}
        -o "${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/generated/"
        -h "${PROJECT_SOURCE_DIR}/include/pbos/generated/syscall/"
        -I "pbos/generated/syscall/"
        -a "${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/arch.json"
        -t "${CMAKE_CURRENT_SOURCE_DIR}/pbcore.json"
    VERBATIM
)
add_library(pbscintf INTERFACE "${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/generated/pbcore.cc")
