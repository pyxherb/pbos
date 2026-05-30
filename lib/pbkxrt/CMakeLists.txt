add_library(pbkxrt STATIC)

file(GLOB SRC *.c *.cc *.h *.asm *.s *.S)
target_sources(pbkxrt PRIVATE ${SRC})

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
target_include_directories(pbkxrt PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pbkxrt PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})
