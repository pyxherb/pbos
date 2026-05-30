file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pboskrnl PRIVATE ${SRC})
target_link_libraries(pboskrnl PUBLIC freestdc pbos_common)
