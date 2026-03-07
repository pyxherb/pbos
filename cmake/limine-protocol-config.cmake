find_path(LIMINE_PROTOCOL_INCLUDE_DIR NAMES "limine.h")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    limine-protocol
    REQUIRED_VARS LIMINE_PROTOCOL_INCLUDE_DIR)
