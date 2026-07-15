find_package(PbOSCommon REQUIRED NO_CMAKE_FIND_ROOT_PATH)

add_library(pbkxrt STATIC)

file(GLOB SRC *.c *.cc *.h *.asm *.s *.S)
target_sources(pbkxrt PRIVATE ${SRC})
target_link_libraries(pbkxrt PRIVATE pbos_common)

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
target_include_directories(pbkxrt PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pbkxrt PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})
