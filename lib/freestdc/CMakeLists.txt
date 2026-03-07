project(freestdc VERSION 0.0.1)

# Setup basic build settings
enable_language(ASM)

include_directories(BEFORE SYSTEM ${PROJECT_SOURCE_DIR}/include)
include_directories(BEFORE SYSTEM ${PROJECT_SOURCE_DIR}/include/cxx)
set_property(GLOBAL PROPERTY FREESTDC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set_property(GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX "${PROJECT_SOURCE_DIR}/include/cxx")

# Library
add_library(freestdc STATIC)

add_subdirectory(include)
add_subdirectory(src)
