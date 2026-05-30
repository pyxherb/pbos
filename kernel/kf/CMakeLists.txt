file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pboskrnl PRIVATE ${SRC})

if (IS_DIRECTORY "./${CMAKE_SYSTEM_PROCESSOR}")
    add_subdirectory("./${CMAKE_SYSTEM_PROCESSOR}")
endif()
