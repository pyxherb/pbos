set(_LOADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/loader/${OICOS_ARCH}")
file(GLOB _children RELATIVE ${_LOADER_SOURCE_DIR} ${_LOADER_SOURCE_DIR}/*)
set(_binldrs "")

foreach(i ${_children})
    if(IS_DIRECTORY ${_LOADER_SOURCE_DIR}/${i})
        list(APPEND _binldrs ${i})
    endif()
endforeach()

set(OICOS_BINLDRS "${_binldrs}" CACHE STRING "Found built-in binary loaders" FORCE)

foreach(i ${OICOS_BINLDRS})
    message("Found binary loader: ${i}")
endforeach()
