set(FREESTDC_ENABLE_SPECIALIZATION true CACHE BOOL "Enable specializations for FreeSTDC")

message(CHECK_START "Finding ${CMAKE_SYSTEM_PROCESSOR} specializations for FreeSTDC")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_PROCESSOR}/CMakeLists.txt")
    if(${FREESTDC_ENABLE_SPECIALIZATION})
        message(CHECK_PASS "enabled")
        add_subdirectory(${CMAKE_SYSTEM_PROCESSOR})
    else()
        message(CHECK_FAIL "disabled")
    endif()
else()
    message(CHECK_FAIL "not found")
endif()

file(GLOB SRC *.c *.h *.asm *.s *.S)
target_sources(freestdc PRIVATE ${SRC})

add_subdirectory("common")
