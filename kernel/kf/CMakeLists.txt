file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbkrnl PRIVATE ${SRC})

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
    add_subdirectory("i386")
endif()
