file(GLOB SRC *.c *.cc *.asm *.s *.S)
file(GLOB HEADERS *.hh *.h)

target_sources(freestdc PRIVATE ${SRC} ${HEADERS})
add_subdirectory("_coroutine")
add_subdirectory("_type_traits")
add_subdirectory("_utility")

install(FILES ${HEADERS} DESTINATION include/cxx)
