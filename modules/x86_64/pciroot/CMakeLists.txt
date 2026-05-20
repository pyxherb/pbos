file(GLOB SRC *.c *.cc *.h *.S)
add_library(pciroot SHARED ${SRC})
target_link_libraries(pciroot PRIVATE pbarch)

target_link_options(pciroot PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kmod.lds")

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
target_include_directories(pciroot PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pciroot PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})
