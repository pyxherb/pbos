file(GLOB_RECURSE SRC *.c *.h *.S)
add_executable(pbinit ${SRC})
target_link_options(pbinit PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/init.lds")

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
target_include_directories(pbinit SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pbinit SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})

add_dependencies(pbinit freestdc)
target_link_libraries(pbinit PUBLIC pbos_common freestdc)
