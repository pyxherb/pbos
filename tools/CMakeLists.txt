cmake_minimum_required(VERSION 3.15)
project(OicOSTools)

# if(NOT DEFINED ${OICOS_TOOLS_BINARY_DIR})
    # set(OICOS_TOOLS_BINARY_DIR ${PROJECT_BINARY_DIR})
# endif()

add_subdirectory("kimgen")
add_subdirectory("cltgen")
add_subdirectory("cargen")
