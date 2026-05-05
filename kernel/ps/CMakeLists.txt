file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbkrnl PRIVATE ${SRC})

add_subdirectory("sched")
