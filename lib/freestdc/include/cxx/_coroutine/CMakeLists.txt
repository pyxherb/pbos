file(GLOB SRC *.cc *.c *.asm *.s *.S)
file(GLOB HEADERS *.hh *.h)

target_sources(freestdc PRIVATE ${SRC} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/cxx/_coroutine)
