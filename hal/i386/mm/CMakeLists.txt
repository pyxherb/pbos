file(GLOB SRC *.h *.c *.S)
target_sources(pbhal PRIVATE ${SRC})

add_subdirectory("pgalloc")
add_subdirectory("vmmgr")
