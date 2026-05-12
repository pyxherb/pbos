file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbkrnl PRIVATE ${SRC})
target_link_libraries(pbkrnl PUBLIC freestdc pbos_common)
