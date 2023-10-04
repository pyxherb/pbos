project(kernel)

file(GLOB_RECURSE SRC *.c *.h *.S)
add_executable(oickim ${SRC})

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
target_include_directories(oickim SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR})

add_dependencies(oickim oicarch freestdc hal bootldr tools)
target_link_libraries(oickim oicarch freestdc hal)
target_link_options(oickim PRIVATE "-T${OICOS_ROOT_DIR}/scripts/ld/${OICOS_ARCH}/kernel.lds")

add_custom_command(
    TARGET oickim POST_BUILD
    COMMAND ${OICOS_TOOLS_BINARY_DIR}/kimgen create ${OICOS_BINARY_DIR}/bootldr $<TARGET_FILE:oickim> ${OICOS_BINARY_DIR}/oickim
    COMMAND ${OICOS_TOOLS_BINARY_DIR}/cargen -c ${OICOS_BINARY_DIR}/initcar _ $<TARGET_FILE:insinit>
    BYPRODUCTS ${OICOS_BINARY_DIR}/oickim ${OICOS_BINARY_DIR}/initcar
    DEPENDS bootldr insinit
    WORKING_DIRECTORY ${OICOS_TOOLS_BINARY_DIR})

add_subdirectory("exec")
add_subdirectory("io")
add_subdirectory("kf")
add_subdirectory("km")
