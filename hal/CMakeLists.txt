file(GLOB SRC *.c *.h)
add_library(pbhal ${SRC})
target_link_libraries(pbhal PUBLIC pbos_common pbkfxx)
add_subdirectory(${PBOS_ARCH})
