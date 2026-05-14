cmake_minimum_required(VERSION 3.23)

# Setup project
project(PbOS VERSION 0.0.1)

set(PBOS_ENABLE_ASLR FALSE CACHE BOOL "Enable ASLR for the kernel")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PbOSCommon REQUIRED NO_CMAKE_FIND_ROOT_PATH)

# Cache variables
set(_GUESSED_ARCH)

if((CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "i386") OR (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "i686") OR (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86"))
    set(_GUESSED_ARCH "i386")
elseif((CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64") OR (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64"))
    set(_GUESSED_ARCH "x86_64")
elseif((CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64") OR (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64"))
    set(_GUESSED_ARCH "aarch64")
else()
    set(_GUESSED_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()

set(PBOS_ARCH ${_GUESSED_ARCH} CACHE STRING "Identifier of target architecture")

# Setup build environments
enable_language(ASM)

if((PBOS_ARCH STREQUAL "i386") OR (PBOS_ARCH STREQUAL "x86_64"))
    enable_language(ASM_NASM)
endif()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include_directories(${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/include")

set(PBOS_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(PBOS_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Subdirectories
add_subdirectory("arch")
add_subdirectory("lib")
include_directories("lib")

add_subdirectory("hal")
add_subdirectory("modules")
add_subdirectory("kernel")

add_subdirectory("test")
add_subdirectory("init")

add_subdirectory("include")
