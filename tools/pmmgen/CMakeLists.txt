project(pmmgen)
file(GLOB SRC *.h *.hh *.c *.cc)

include_directories(${OICOS_SOURCE_DIR})
include_directories("${OICOS_SOURCE_DIR}/include")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

bison_target(pmmparse pmmparse.yy ${CMAKE_CURRENT_BINARY_DIR}/pmmparse.cc COMPILE_FLAGS)
flex_target(pmmlex pmmlex.ll ${CMAKE_CURRENT_BINARY_DIR}/pmmlex.cc COMPILE_FLAGS)

add_executable(pmmgen ${BISON_pmmparse_OUTPUT_SOURCE} ${FLEX_pmmlex_OUTPUTS} ${SRC})
set_target_properties(
    pmmgen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OICOS_TOOLS_BINARY_DIR}
)
set_target_properties(
    pmmgen
    PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "${OICOS_BINARY_DIR}/bootldr ${OICOS_BINARY_DIR}/oickim ${OICOS_BINARY_DIR}/oickim"
)

target_compile_definitions(pmmgen PRIVATE -D_CRT_SECURE_NO_WARNINGS)
