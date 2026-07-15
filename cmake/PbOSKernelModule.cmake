function(add_pbos_kernel_module Name)
    message("Added PbOS kernel module: ${Name}")

	string(RANDOM OutputFileName)
	add_library(
		${Name}
		SHARED
	)

	if(PROJECT_NAME STREQUAL "PbOS")
		target_link_libraries(${Name} PRIVATE pbkxrt)
		target_link_options(${Name} PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kmod.lds")
		get_property(_modules GLOBAL PROPERTY PBOS_KERNEL_MODULES)
		list(APPEND _modules ${Name})
		set_property(GLOBAL PROPERTY PBOS_KERNEL_MODULES ${_modules})

		get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
		get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
		target_include_directories(${Name} PRIVATE ${FREESTDC_INCLUDE_DIR})
		target_include_directories(${Name} PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})
	else()
		# TODO: Implement it.
	endif()
endfunction()
