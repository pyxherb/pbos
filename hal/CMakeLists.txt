project(hal)

file(GLOB SRC *.c *.h)
add_library(hal ${SRC})
add_subdirectory(${OICOS_ARCH})
