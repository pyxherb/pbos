if(CMAKE_HOST_WIN32)
    add_custom_target(bootimg
        powershell -ExecutionPolicy RemoteSigned ${CMAKE_CURRENT_SOURCE_DIR}/mkimg.ps1
        DEPENDS oickim
        BYPRODUCTS boot.vhd
        WORKING_DIRECTORY ${OICOS_SOURCE_DIR})
elseif(CMAKE_HOST_UNIX)
    add_custom_target(bootimg
        ${CMAKE_CURRENT_SOURCE_DIR}/mkimg.sh
        DEPENDS oickim
        BYPRODUCTS boot.raw
        WORKING_DIRECTORY ${OICOS_SOURCE_DIR})
endif()
