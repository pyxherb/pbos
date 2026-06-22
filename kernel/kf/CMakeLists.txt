file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pboskrnl PRIVATE ${SRC})

if (IS_DIRECTORY "./${PBOS_ARCH}")
    add_subdirectory("./${PBOS_ARCH}")
endif()
