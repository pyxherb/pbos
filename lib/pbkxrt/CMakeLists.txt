add_library(pbkxrt STATIC)

file(GLOB SRC *.c *.cc *.h *.asm *.s *.S)
target_sources(pbkxrt PRIVATE ${SRC})
