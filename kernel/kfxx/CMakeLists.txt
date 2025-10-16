add_library(pbkfxx)

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
target_include_directories(pbkfxx PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pbkfxx PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})

file(GLOB SRC *.hh *.cc *.h *.c *.S)
target_sources(pbkfxx PRIVATE ${SRC})
target_link_libraries(pbkfxx PUBLIC freestdc pbos_common)
