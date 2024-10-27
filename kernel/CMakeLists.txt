project(kernel)

file(GLOB_RECURSE SRC *.c *.h *.S)
add_executable(pbkim ${SRC})

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
target_include_directories(pbkim SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR})

add_dependencies(pbkim pbarch freestdc hal bootldr tools)
target_link_libraries(pbkim pbarch freestdc hal)
target_link_options(pbkim PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds")

add_custom_command(
    TARGET pbkim POST_BUILD
    COMMAND ${PBOS_TOOLS_BINARY_DIR}/kimgen create ${PBOS_BINARY_DIR}/bootldr $<TARGET_FILE:pbkim> ${PBOS_BINARY_DIR}/pbkim
    COMMAND ${PBOS_TOOLS_BINARY_DIR}/cargen -c ${PBOS_BINARY_DIR}/initcar _ $<TARGET_FILE:pbinit>
    BYPRODUCTS ${PBOS_BINARY_DIR}/pbkim ${PBOS_BINARY_DIR}/initcar
    DEPENDS bootldr pbinit
    WORKING_DIRECTORY ${PBOS_TOOLS_BINARY_DIR})

add_subdirectory("exec")
add_subdirectory("io")
add_subdirectory("kf")
add_subdirectory("km")
