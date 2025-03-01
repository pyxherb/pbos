file(GLOB SRC *.h *.c *.S)
target_sources(pbkern PRIVATE ${SRC})

add_subdirectory("sched")
