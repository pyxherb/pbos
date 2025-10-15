file(GLOB SRC *.c *.cc *.asm *.s *.S)
file(GLOB HEADERS *.hh *.h)

target_sources(freestdc PRIVATE ${SRC} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/cxx/_utility)
