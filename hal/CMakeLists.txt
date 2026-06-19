if(PBOS_ENABLE_KASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=kernel-address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=kernel-address")
endif()

file(GLOB SRC *.c *.h)
add_library(pbhal ${SRC} "${CMAKE_CURRENT_SOURCE_DIR}/${PBOS_ARCH}/generated/pbcore.cc")
target_link_libraries(pbhal PUBLIC pbos_common)
add_subdirectory(${PBOS_ARCH})
