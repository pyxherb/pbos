file(GLOB SRC *.h *.c *.S)
target_sources(hal PRIVATE ${SRC})

add_subdirectory("kima")
add_subdirectory("pgalloc")
add_subdirectory("vmmgr")
