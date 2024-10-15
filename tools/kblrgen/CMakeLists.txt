project(kbblgen)
file(GLOB SRC *.h *.c)

include_directories(${PBOS_SOURCE_DIR})
include_directories("${PBOS_SOURCE_DIR}/include")

add_executable(kbblgen ${SRC})
set_target_properties(
    kbblgen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PBOS_TOOLS_BINARY_DIR}
)

target_compile_definitions(kbblgen PRIVATE -D_CRT_SECURE_NO_WARNINGS)
