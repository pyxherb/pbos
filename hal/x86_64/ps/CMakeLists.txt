file(GLOB SRC *.h *.hh *.c *.cc *.asm *.S)
target_sources(pbhal PRIVATE ${SRC})
