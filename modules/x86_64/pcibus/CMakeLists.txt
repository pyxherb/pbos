file(GLOB SRC *.c *.cc *.h *.S)
add_pbos_kernel_module(pcibus)
target_sources(pcibus PRIVATE ${SRC})
target_link_libraries(pcibus PRIVATE pbarch)

