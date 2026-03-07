find_package(limine-protocol REQUIRED)

target_sources(pbhal PRIVATE "main.c")

target_include_directories(pbhal PUBLIC ${LIMINE_PROTOCOL_INCLUDE_DIR})
