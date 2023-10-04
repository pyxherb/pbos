project(kimgen)
file(GLOB SRC *.h *.c)

include_directories(${OICOS_SOURCE_DIR})
include_directories("${OICOS_SOURCE_DIR}/include")

add_executable(kimgen ${SRC})
set_target_properties(
    kimgen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OICOS_TOOLS_BINARY_DIR}
)
set_target_properties(
    kimgen
    PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "${OICOS_BINARY_DIR}/bootldr ${OICOS_BINARY_DIR}/oickim ${OICOS_BINARY_DIR}/oickim"
)

target_compile_definitions(kimgen PRIVATE -D_CRT_SECURE_NO_WARNINGS)
