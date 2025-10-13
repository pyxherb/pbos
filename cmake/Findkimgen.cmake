if(CMAKE_HOST_WIN32)
	foreach(i ${CMAKE_SYSTEM_PREFIX_PATH})
		message(CHECK_START "Finding kimgen: ${i}/PbOSTools/bin")

		find_program(KIMGEN_EXECUTABLE kimgen NAMES kimgen HINTS ${i}/PbOSTools/bin)

		if (KIMGEN_EXECUTABLE)
			message(CHECK_PASS "Found kimgen: ${KIMGEN_EXECUTABLE}")
			break()
		endif()
	endforeach()
else()
	find_program(KIMGEN_EXECUTABLE kimgen NAMES kimgen)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    kimgen
    REQUIRED_VARS KIMGEN_EXECUTABLE)
