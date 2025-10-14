cmake_minimum_required(VERSION 3.10)
include(../../../../nncase/examples/cmake/Riscv64.cmake)

set(CMAKE_BUILD_TYPE Release)

project(sample-lvgl-nxp)
set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../../../nncase")
set(k230_sdk ${nncase_sdk_root}/../../../)
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../" CACHE STRING "The path to use for make install" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/../linker_scripts/riscv64/link.lds --static")
add_definitions(-march=rv64imafdcv -mcmodel=medany -mabi=lp64d -n --static -DHAVE_CCONFIG_H)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
endif()
#set(lvgl_src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lvgl-9.2.1)
#set(lvgl_src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lvgl)
set(lvgl_src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lvgl_nxp)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../api
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/comm
    $ENV{RTSMART_SRC_DIR}/userapps/sdk/rt-thread/include
    $ENV{RTSMART_SRC_DIR}/userapps/sdk/rt-thread/components/drivers
    $ENV{RTSMART_SRC_DIR}/userapps/
    ${lvgl_src}
)
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
)
set(LV_CONF_BUILD_DISABLE_EXAMPLES TRUE CACHE BOOL "Disable building LVGL examples.")
set(LV_CONF_BUILD_DISABLE_DEMOS TRUE CACHE BOOL "Disable building LVGL demos.")

add_subdirectory(${lvgl_src} ${CMAKE_CURRENT_BINARY_DIR}/lvgl)
set(nxp_ui_name nxp)
#set(nxp_ui_name nxp1)

file(GLOB_RECURSE
    NXP_SOURCES
    CONFIGURE_DEPENDS
    ${nxp_ui_name}/**/*.c
)
include_directories(
    ${nxp_ui_name}/custom
    ${nxp_ui_name}/generated
)
#message(STATUS "wjxSOURCES 的值是: ${SOURCES}")
add_executable(${PROJECT_NAME}.elf main.c uart_rcv.c uart_ui.c ring_buffer.c ${NXP_SOURCES})


if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    #target_link_options(${PROJECT_NAME}.elf PRIVATE -mcpu=c908v)
    if( supported )
        message(STATUS "IPO / LTO enabled")
        set_property(TARGET ${PROJECT_NAME}.elf PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET lvgl PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
endif()

#target_link_libraries(${PROJECT_NAME}.elf lvgl lvgl::examples lvgl::demos lvgl::thorvg vg_lite vg_lite_util m vo connector vb sys)
#target_link_libraries(${PROJECT_NAME}.elf lvgl lvgl::thorvg vg_lite vg_lite_util m vo connector vb sys)
target_link_libraries(${PROJECT_NAME}.elf lvgl lvgl::thorvg vg_lite vg_lite_util m vo connector vb sys)
install(TARGETS ${PROJECT_NAME}.elf DESTINATION elf)
