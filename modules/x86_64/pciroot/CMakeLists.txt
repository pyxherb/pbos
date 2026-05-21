file(GLOB SRC *.c *.cc *.h *.S)
add_pbos_kernel_module(pciroot)
target_sources(pciroot PRIVATE ${SRC})
target_link_libraries(pciroot PRIVATE pbarch)

