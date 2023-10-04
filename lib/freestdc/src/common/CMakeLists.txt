file(GLOB SRC *.c *.h *.asm *.s *.S)

target_sources(freestdc PRIVATE ${SRC})
