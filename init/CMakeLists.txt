project(insinit)

file(GLOB_RECURSE SRC *.c *.h *.S)
add_executable(insinit ${SRC})
target_link_options(pbkim PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/init.lds")

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
target_include_directories(insinit SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR})

add_dependencies(insinit freestdc)
target_link_libraries(insinit freestdc)
