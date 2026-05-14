target_sources(pbhal PRIVATE "boot.cc")

target_include_directories(pbhal PUBLIC ${LIMINE_PROTOCOL_INCLUDE_DIR})

# target_link_libraries(pbhal PRIVATE pbhal_reloc)
