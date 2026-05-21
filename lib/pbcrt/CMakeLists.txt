project(pbcrt VERSION 0.0.1)

# Setup basic build settings
enable_language(ASM)

# Library
add_library(pbcrt STATIC)

add_subdirectory("src")
