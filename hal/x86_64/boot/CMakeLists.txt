include(FetchContent)

FetchContent_Declare(
    LimineProtocol
    GIT_REPOSITORY https://github.com/Limine-Bootloader/limine-protocol
    GIT_TAG        trunk
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(
    LimineProtocol
    SOURCE_DIR LimineProtocol_SOURCE_DIR
    POPULATED LimineProtocol_POPULATED)
if(NOT LimineProtocol_POPULATED)
    message("Fetching limine-protocol")
    FetchContent_MakeAvailable(LimineProtocol)
endif()

FetchContent_GetProperties(
    LimineProtocol
    SOURCE_DIR LimineProtocol_SOURCE_DIR)

target_sources(pbhal PRIVATE "boot.cc")

target_include_directories(pbhal PUBLIC ${LimineProtocol_SOURCE_DIR}/include)

# target_link_libraries(pbhal PRIVATE pbhal_reloc)
