if(CMAKE_HOST_WIN32)
	foreach(i ${CMAKE_SYSTEM_PREFIX_PATH})
		message(CHECK_START "Finding cargen: ${i}/PbOSTools/bin")

		find_program(CARGEN_EXECUTABLE cargen NAMES cargen HINTS ${i}/PbOSTools/bin)

		if (CARGEN_EXECUTABLE)
			message(CHECK_PASS "Found cargen: ${CARGEN_EXECUTABLE}")
			break()
		endif()
	endforeach()
else()
	find_program(CARGEN_EXECUTABLE cargen NAMES cargen)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    cargen
    REQUIRED_VARS CARGEN_EXECUTABLE)
