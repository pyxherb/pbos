cmake_minimum_required(VERSION 3.15)

include(ExternalProject)

# Cache variables
set(PBOS_ARCH "i386" CACHE STRING "Identifier of target architecture")

# Setup project
project(PBOS VERSION 0.0.1)

# Setup build environments
enable_language(ASM)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/scripts/cmake")
set(CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/scripts/cmake")
set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/scripts/cmake")

include_directories(${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/include")

set(PBOS_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(PBOS_TOOLS_BINARY_DIR "${PBOS_BINARY_DIR}/tools")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM "${PBOS_BINARY_DIR}/tools/bin")

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
    SOURCE_DIR ${PBOS_SOURCE_DIR}/tools
    BINARY_DIR ${PBOS_TOOLS_BINARY_DIR}/bin
    PREFIX ${PBOS_BINARY_DIR}/tools
    CMAKE_ARGS
        # Use native toolchain
        -UCMAKE_TOOLCHAIN_FILE
        # Inherited variables
        -DPBOS_SOURCE_DIR=${PBOS_SOURCE_DIR}
        -DPBOS_TOOLS_BINARY_DIR=${PBOS_TOOLS_BINARY_DIR}
    BUILD_ALWAYS true
    # Ignore installations
    INSTALL_COMMAND ${CMAKE_COMMAND} -E true
)
