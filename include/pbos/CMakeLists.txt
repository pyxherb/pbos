set(PBOS_ORDER_MAX 7 CACHE STRING "Maximum order for page blocks")
set(PBOS_PROC_MAX 32767 CACHE STRING "Maximum process count")
set(PBOS_SCHEDULER_NAME 32767 CACHE STRING "Scheduler name")

function(add_config_header filepath)
    string(REGEX MATCH "([a-zA-Z0-9/.]+).in" filepath ${filepath})
    message("Added configuration header: ${filepath}")
    configure_file(
        config/${filepath}
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/${CMAKE_MATCH_1}
    )
endfunction()

add_config_header("km.h.in")
