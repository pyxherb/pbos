project(kernel)

file(GLOB SRC *.c *.h *.S)
add_executable(pbkern ${SRC})

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
target_include_directories(pbkern SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR})

add_dependencies(pbkern pbarch freestdc hal bootldr tools)
target_link_libraries(pbkern pbarch freestdc hal)
target_link_options(pbkern PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds")

add_custom_target(
    pbkim
    COMMAND ${PBOS_TOOLS_BINARY_DIR}/kimgen create ${PBOS_BINARY_DIR}/bootldr $<TARGET_FILE:pbkern> ${PBOS_BINARY_DIR}/pbkim
    BYPRODUCTS ${PBOS_BINARY_DIR}/pbkim
    DEPENDS bootldr pbinit
    WORKING_DIRECTORY ${PBOS_TOOLS_BINARY_DIR})

add_custom_target(
    initcar
    COMMAND ${PBOS_TOOLS_BINARY_DIR}/cargen -c ${PBOS_BINARY_DIR}/initcar _ $<TARGET_FILE:pbinit>
    BYPRODUCTS ${PBOS_BINARY_DIR}/initcar
    DEPENDS pbkim
    WORKING_DIRECTORY ${PBOS_TOOLS_BINARY_DIR}
)

add_subdirectory("exec")
add_subdirectory("io")
add_subdirectory("kf")
add_subdirectory("km")
add_subdirectory("mm")
add_subdirectory("fs")
add_subdirectory("ps")
