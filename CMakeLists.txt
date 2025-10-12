cmake_minimum_required(VERSION 3.23)

# Setup project
project(PbOS VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PbOSCommon REQUIRED NO_CMAKE_FIND_ROOT_PATH)

# Cache variables
set(PBOS_ARCH "i386" CACHE STRING "Identifier of target architecture")

# Setup build environments
enable_language(ASM)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include_directories(${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/include")

set(PBOS_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(PBOS_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
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
