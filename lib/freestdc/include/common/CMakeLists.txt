file(GLOB SRC *.c *.asm *.s *.S)
file(GLOB HEADERS *.h)

target_sources(freestdc PRIVATE ${SRC} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/common/)
