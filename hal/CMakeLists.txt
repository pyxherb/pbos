file(GLOB SRC *.c *.h)
add_library(hal ${SRC})
target_link_libraries(hal PUBLIC pbos_common)
add_subdirectory(${PBOS_ARCH})
