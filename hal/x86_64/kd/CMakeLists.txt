find_package(file2c REQUIRED)

file(GLOB SRC *.h *.hh *.c *.cc *.S)

add_file2c_target(
    TARGET_NAME pbhal_target_xml_inclusion
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/target.xml
    OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/target.xml.cc
    VAR_NAME kh_gdb_target_xml
    PREFIXES ""
    INCLUDE_FILES "<hal/x86_64/proc.hh>"
)
list(APPEND SRC ${CMAKE_CURRENT_BINARY_DIR}/target.xml.cc)

target_sources(pbhal PRIVATE ${SRC})
