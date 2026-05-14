get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)

file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pbhal PRIVATE ${SRC})
target_include_directories(pbhal SYSTEM BEFORE PRIVATE ${FREESTDC_INCLUDE_DIR} ${FREESTDC_INCLUDE_DIR_CXX})
target_link_libraries(pbhal PUBLIC pbarch freestdc pbscintf)

add_subdirectory("boot")
