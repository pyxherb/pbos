project(fmtctl VERSION 0.0.1)

include_directories(${PROJECT_SOURCE_DIR})

# Library
add_library(fmtctl STATIC "fmtctl.h" "fmtctl.c")

# Installation
install(TARGETS fmtctl
		EXPORT ${PROJECT_NAME}Targets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		PUBLIC_HEADER DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	VERSION ${PACKAGE_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(EXPORT ${PROJECT_NAME}Targets
		FILE ${PROJECT_NAME}Targets.cmake
		DESTINATION "lib/cmake/${PROJECT_NAME}")

configure_package_config_file(
	${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake
	INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
			"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
		DESTINATION "lib/cmake/${PROJECT_NAME}")
