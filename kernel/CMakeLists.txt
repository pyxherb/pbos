file(GLOB SRC *.c *.cc *.h *.S)
add_executable(pbkrnl ${SRC})

#find_package(kimgen REQUIRED)
find_package(cargen REQUIRED)

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
target_include_directories(pbkrnl PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pbkrnl PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})

# add_dependencies(pbkrnl bootldr)
target_link_libraries(pbkrnl PRIVATE pbarch freestdc pbhal)
target_compile_definitions(pbkrnl PRIVATE PBOS_COMPILING_KERNEL=1)
target_link_options(pbkrnl PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds")

#add_custom_target(
#    pbkim
#    COMMAND ${KIMGEN_EXECUTABLE} --machine-i386 --bin-file ${PBOS_BINARY_DIR}/bootldr --kimage-file $<TARGET_FILE:pbkrnl> -o ${PBOS_BINARY_DIR}/pbkim
#    BYPRODUCTS ${PBOS_BINARY_DIR}/pbkim
#    DEPENDS bootldr pbinit)

set(KERNEL_MODULES_TARGET_FILES "")

get_property(_KERNEL_MODULES GLOBAL PROPERTY KERNEL_MODULES)

foreach(i ${_KERNEL_MODULES})
    if(_KERNEL_MODULES STREQUAL "")
        set(KERNEL_MODULES_TARGET_FILES "${KERNEL_MODULES_TARGET_FILES} $<TARGET_FILE:${i}>")
    else()
        set(KERNEL_MODULES_TARGET_FILES "$<TARGET_FILE:${i}>")
    endif()
endforeach()

foreach(i ${_KERNEL_MODULES})
    set(KERNEL_MODULES_TARGETS ${KERNEL_MODULES_TARGETS} ${i})
endforeach()

add_custom_target(
    initcar
    COMMAND ${CARGEN_EXECUTABLE} -c ${PBOS_BINARY_DIR}/initcar _ $<TARGET_FILE:pbinit> ${KERNEL_MODULES_TARGET_FILES}
    BYPRODUCTS ${PBOS_BINARY_DIR}/initcar
    DEPENDS pbkrnl pbinit ${KERNEL_MODULES_TARGETS}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds.ph.cc
    COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds.ph.cc
    DEPENDS "${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds"
)
target_sources(pbkrnl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds.ph.cc)

set(PBOS_INITIAL_MM_AREA_STORAGE_NUM 256 CACHE STRING "Number of initial memory area storage")

configure_file(
    "config.hh.in"
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/config.hh
)

add_subdirectory("io")
add_subdirectory("kf")
add_subdirectory("kfxx")
add_subdirectory("km")
add_subdirectory("kd")
add_subdirectory("mm")
add_subdirectory("fs")
add_subdirectory("ps")
add_subdirectory("syscall")
add_subdirectory("acpi")
add_subdirectory("mp")
add_subdirectory("dm")
add_subdirectory("initcar")
