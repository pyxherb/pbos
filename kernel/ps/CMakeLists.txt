file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbkern PRIVATE ${SRC})

add_subdirectory("sched")
