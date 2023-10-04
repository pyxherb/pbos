file(GLOB SRC *.h *.c *.S)
target_sources(hal PRIVATE ${SRC})

add_subdirectory("skid")
add_subdirectory("pgalloc")
add_subdirectory("vmmgr")
