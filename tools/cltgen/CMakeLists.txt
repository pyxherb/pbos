project(cltgen)
file(GLOB SRC *.h *.hh *.c *.cc)

include_directories(${PBOS_SOURCE_DIR})
include_directories("${PBOS_SOURCE_DIR}/include")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

bison_target(
    cltparse
    cltparse.yy ${CMAKE_CURRENT_BINARY_DIR}/cltparse.cc
    COMPILE_FLAGS)
flex_target(
    cltlex
    cltlex.ll ${CMAKE_CURRENT_BINARY_DIR}/cltlex.cc
    COMPILE_FLAGS
    "--header-file=${CMAKE_CURRENT_BINARY_DIR}/cltlex.h --nounistd")

add_executable(cltgen ${BISON_cltparse_OUTPUT_SOURCE} ${FLEX_cltlex_OUTPUTS} ${SRC})
set_target_properties(
    cltgen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PBOS_TOOLS_BINARY_DIR}
)
set_target_properties(
    cltgen
    PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "${PBOS_BINARY_DIR}/bootldr ${PBOS_BINARY_DIR}/pbkim ${PBOS_BINARY_DIR}/pbkim"
)

target_compile_definitions(cltgen PRIVATE -D_CRT_SECURE_NO_WARNINGS)
