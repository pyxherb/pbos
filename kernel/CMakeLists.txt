file(GLOB SRC *.c *.cc *.h *.S)
add_executable(pbkern ${SRC})

find_package(kimgen REQUIRED)
find_package(cargen REQUIRED)

get_property(FREESTDC_INCLUDE_DIR GLOBAL PROPERTY FREESTDC_INCLUDE_DIR)
get_property(FREESTDC_INCLUDE_DIR_CXX GLOBAL PROPERTY FREESTDC_INCLUDE_DIR_CXX)
target_include_directories(pbkern PRIVATE ${FREESTDC_INCLUDE_DIR})
target_include_directories(pbkern PRIVATE ${FREESTDC_INCLUDE_DIR_CXX})

add_dependencies(pbkern pbarch freestdc hal bootldr)
target_link_libraries(pbkern PRIVATE pbarch freestdc hal)
target_link_options(pbkern PRIVATE "-T${PBOS_ROOT_DIR}/scripts/ld/${PBOS_ARCH}/kernel.lds")

add_custom_target(
    pbkim
    COMMAND ${KIMGEN_EXECUTABLE} create ${PBOS_BINARY_DIR}/bootldr $<TARGET_FILE:pbkern> ${PBOS_BINARY_DIR}/pbkim
    BYPRODUCTS ${PBOS_BINARY_DIR}/pbkim
    DEPENDS bootldr pbinit)

add_custom_target(
    initcar
    COMMAND ${CARGEN_EXECUTABLE} -c ${PBOS_BINARY_DIR}/initcar _ $<TARGET_FILE:pbinit>
    BYPRODUCTS ${PBOS_BINARY_DIR}/initcar
    DEPENDS pbkim
)

add_subdirectory("exec")
add_subdirectory("io")
add_subdirectory("kf")
add_subdirectory("km")
add_subdirectory("mm")
add_subdirectory("fs")
add_subdirectory("ps")
