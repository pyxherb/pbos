file(GLOB SRC *.c *.cc *.h *.S)
add_pbos_kernel_module(i8042)
target_sources(i8042 PRIVATE ${SRC})

