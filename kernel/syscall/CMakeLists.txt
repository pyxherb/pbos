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
        -i "pbos/syscall/"
        -I "pbos/syscall/_pbcore_pre.h"
        --name hali_pbcore_dispatcher
        -a "${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/arch.json"
        -t "${CMAKE_CURRENT_SOURCE_DIR}/pbcore.json"
    DEPENDS
        ${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/arch.json
        ${CMAKE_CURRENT_SOURCE_DIR}/pbcore.json
    VERBATIM
)
add_library(pbscintf INTERFACE "${PROJECT_SOURCE_DIR}/hal/${PBOS_ARCH}/generated/pbcore.cc")
