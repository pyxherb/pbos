find_package(dcgen REQUIRED)

add_dcgen_cxx_target(pboskrnl_devcls ${CMAKE_CURRENT_SOURCE_DIR}/classes/devcls.json "${PROJECT_SOURCE_DIR}/include/pbos/generated/dm/" devcls)

file(GLOB SRC *.h *.hh *.c *.cc *.S)
target_sources(pboskrnl PRIVATE ${SRC})
target_link_libraries(pboskrnl PRIVATE pboskrnl_devcls)

