file(GLOB SRC *.h *.c *.S)
target_sources(oickim PRIVATE ${SRC})

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
    add_subdirectory("i386")
endif()
