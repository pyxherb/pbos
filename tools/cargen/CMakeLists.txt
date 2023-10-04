project(cargen)
file(GLOB SRC *.h *.c)

include_directories(${OICOS_SOURCE_DIR})
include_directories("${OICOS_SOURCE_DIR}/include")

add_executable(cargen ${SRC})
set_target_properties(
    cargen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OICOS_TOOLS_BINARY_DIR}
)

target_compile_definitions(cargen PRIVATE -D_CRT_SECURE_NO_WARNINGS)
