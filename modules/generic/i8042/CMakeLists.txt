file(GLOB SRC *.c *.cc *.h *.S)
add_pbos_kernel_module(kbdcls)
target_sources(kbdcls PRIVATE ${SRC})

