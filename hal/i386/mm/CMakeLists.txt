file(GLOB SRC *.hh *.cc *.h *.hh *.c *.cc *.S)
target_sources(pbhal PRIVATE ${SRC})

add_subdirectory("pgalloc")
add_subdirectory("vmmgr")
