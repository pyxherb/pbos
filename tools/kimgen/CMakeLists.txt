project(kimgen)
file(GLOB SRC *.h *.c)

include_directories(${PBOS_SOURCE_DIR})
include_directories("${PBOS_SOURCE_DIR}/include")

add_executable(kimgen ${SRC})
set_target_properties(
    kimgen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PBOS_TOOLS_BINARY_DIR}
)
set_target_properties(
    kimgen
    PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "${PBOS_BINARY_DIR}/bootldr ${PBOS_BINARY_DIR}/pbkim ${PBOS_BINARY_DIR}/pbkim"
)

target_compile_definitions(kimgen PRIVATE -D_CRT_SECURE_NO_WARNINGS)
