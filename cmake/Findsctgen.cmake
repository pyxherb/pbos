if(CMAKE_HOST_WIN32)
	foreach(i ${CMAKE_SYSTEM_PREFIX_PATH})
		message(CHECK_START "Finding sctgen: ${i}/PbOSTools/bin")

		find_program(SCTGEN_EXECUTABLE sctgen NAMES sctgen HINTS ${i}/PbOSTools/bin)

		if (SCTGEN_EXECUTABLE)
			message(CHECK_PASS "Found sctgen: ${SCTGEN_EXECUTABLE}")
			break()
		endif()
	endforeach()
else()
	find_program(SCTGEN_EXECUTABLE sctgen NAMES sctgen)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    sctgen
    REQUIRED_VARS SCTGEN_EXECUTABLE)
