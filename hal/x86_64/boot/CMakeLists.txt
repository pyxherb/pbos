find_package(limine-protocol REQUIRED)

target_sources(pbhal PRIVATE "main.cc")

target_include_directories(pbhal PUBLIC ${LIMINE_PROTOCOL_INCLUDE_DIR})
