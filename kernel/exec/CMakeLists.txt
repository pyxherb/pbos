set(_LOADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/loader/${CMAKE_SYSTEM_PROCESSOR}")
file(GLOB _children RELATIVE ${_LOADER_SOURCE_DIR} ${_LOADER_SOURCE_DIR}/*)
set(_binldrs "")

foreach(i ${_children})
    if(IS_DIRECTORY ${_LOADER_SOURCE_DIR}/${i})
        list(APPEND _binldrs ${i})
    endif()
endforeach()

set(PBOS_BINLDRS "${_binldrs}" CACHE STRING "Found built-in binary loaders" FORCE)

foreach(i ${PBOS_BINLDRS})
    message("Found binary loader: ${i}")
    add_subdirectory(${_LOADER_SOURCE_DIR}/${i})
endforeach()

file(GLOB SRC *.h *.c *.S)
target_sources(pbkern PRIVATE ${SRC})
