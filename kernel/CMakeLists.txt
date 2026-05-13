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
target_link_options(pbkrnl PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds" "--export-dynamic")

#add_custom_target(
#    pbkim
#    COMMAND ${KIMGEN_EXECUTABLE} --machine-i386 --bin-file ${PBOS_BINARY_DIR}/bootldr --kimage-file $<TARGET_FILE:pbkrnl> -o ${PBOS_BINARY_DIR}/pbkim
#    BYPRODUCTS ${PBOS_BINARY_DIR}/pbkim
#    DEPENDS bootldr pbinit)

add_custom_target(
    initcar
    COMMAND ${CARGEN_EXECUTABLE} -c ${PBOS_BINARY_DIR}/initcar _ $<TARGET_FILE:pbinit>
    BYPRODUCTS ${PBOS_BINARY_DIR}/initcar
    DEPENDS pbkrnl pbinit
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds.ph.cc
    COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds.ph.cc
    DEPENDS "${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds"
)
target_sources(pbkrnl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds.ph.cc)

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
