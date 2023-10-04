cmake_minimum_required(VERSION 3.15)

include(ExternalProject)

# Cache variables
set(OICOS_ARCH "i386" CACHE STRING "Identifier of target architecture")

# Setup project
project(OICOS VERSION 0.0.1)

# Setup build environments
enable_language(ASM)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/scripts/cmake")
set(CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/scripts/cmake")
set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/scripts/cmake")

include_directories(${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/include")

set(OICOS_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(OICOS_TOOLS_BINARY_DIR "${OICOS_BINARY_DIR}/tools")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM "${OICOS_BINARY_DIR}/tools/bin")

# Subdirectories
add_subdirectory("arch")
add_subdirectory("lib")
include_directories("lib")

add_subdirectory("hal")
add_subdirectory("kernel")

add_subdirectory("test")
add_subdirectory("init")

add_subdirectory("include")

ExternalProject_Add(tools
    SOURCE_DIR ${OICOS_SOURCE_DIR}/tools
    BINARY_DIR ${OICOS_TOOLS_BINARY_DIR}/bin
    PREFIX ${OICOS_BINARY_DIR}/tools
    CMAKE_ARGS
        # Use native toolchain
        -UCMAKE_TOOLCHAIN_FILE
        # Inherited variables
        -DOICOS_SOURCE_DIR=${OICOS_SOURCE_DIR}
        -DOICOS_TOOLS_BINARY_DIR=${OICOS_TOOLS_BINARY_DIR}
    BUILD_ALWAYS true
    # Ignore installations
    INSTALL_COMMAND ${CMAKE_COMMAND} -E true
)
