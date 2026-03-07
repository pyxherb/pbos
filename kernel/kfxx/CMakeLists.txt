file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbkern PRIVATE ${SRC})
target_link_libraries(pbkern PUBLIC freestdc pbos_common)
