cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 20)
project(dspp_test_cmdline)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
set(DSPP_INCLUDE "../../dspp/build/dspp")
set(DSPP_LIBRARY_LIB "${CMAKE_SOURCE_DIR}../../dspp/build/dspp" CACHE PATH "Path to dspp library")
set(CMAKE_BUILD_TYPE Debug)
add_executable(dspp_test_cmdline code/main.cpp)

target_link_libraries(dspp_test_cmdline PRIVATE
        ${CMAKE_SOURCE_DIR}/../../dspp/build/dspp/libs/libdspp.a
        ${CMAKE_SOURCE_DIR}/../../dspp/3rdparty/hidapi/x64/debug/hidapi.a
        "-framework CoreFoundation"
        "-framework IOKit"
)

target_include_directories(dspp_test_cmdline PRIVATE
        ${DSPP_INCLUDE}/include
)
else()
    message("This is for LINUX builds only! Run this on linux")
endif()