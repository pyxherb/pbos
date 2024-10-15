cmake_minimum_required(VERSION 3.15)
project(PbOSTools)

# if(NOT DEFINED ${PBOS_TOOLS_BINARY_DIR})
    # set(PBOS_TOOLS_BINARY_DIR ${PROJECT_BINARY_DIR})
# endif()

add_subdirectory("kimgen")
add_subdirectory("cltgen")
add_subdirectory("cargen")
