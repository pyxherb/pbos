if(CMAKE_HOST_WIN32)
	foreach(i ${CMAKE_SYSTEM_PREFIX_PATH})
		message(CHECK_START "Finding dcgen: ${i}/PbOSTools/bin")

		find_program(DCGEN_EXECUTABLE dcgen NAMES dcgen HINTS ${i}/PbOSTools/bin)

		if (DCGEN_EXECUTABLE)
			message(CHECK_PASS "Found dcgen: ${DCGEN_EXECUTABLE}")
			break()
		endif()
	endforeach()
else()
	message(CHECK_START "Finding dcgen...")

	find_program(DCGEN_EXECUTABLE dcgen NAMES dcgen)

	if (DCGEN_EXECUTABLE)
		message(CHECK_PASS "Found dcgen: ${DCGEN_EXECUTABLE}")
	endif()
endif()

if(DCGEN_EXECUTABLE)
    macro(add_dcgen_cxx_target name dcgenInput dcgenOutputBaseDir dcgenOutputName)
        add_custom_command(
			OUTPUT ${dcgenOutputBaseDir}/${dcgenOutputName}.h
            COMMAND ${DCGEN_EXECUTABLE} ${dcgenInput} -d ${dcgenOutputBaseDir} -o ${dcgenOutputName} -l cpp
            DEPENDS ${dcgenInput}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
		add_library(
			${name}
			INTERFACE
		)
		target_sources(${name} PUBLIC ${dcgenOutputBaseDir}/${dcgenOutputName}.h)
    endmacro()
endif()
