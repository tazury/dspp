cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 20)
project(dspp)
set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(POSTBUILD_SCRIPT "${CMAKE_SOURCE_DIR}/instructions/postbuild.sh")

# Define your build output directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/code")
set(BUILD_DIR "${CMAKE_BINARY_DIR}")
set(TPTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")


add_definitions(-DDSBASEDIR="${CMAKE_CURRENT_SOURCE_DIR}/code")

    # Add your source directory
    add_library(dspp STATIC
            code/DSPPInfo.h
            code/DualSense.cpp
            code/DualSense.h
            code/DualSenseChecksum.cpp
            code/DualSenseChecksum.h
            code/DualSenseInput.cpp
            code/DualSenseInput.h
            code/DualSenseMgr.cpp
            code/DualSenseMgr.h
            code/DualSenseOutput.cpp
            code/DualSenseOutput.h
            code/Logger.cpp
            code/Logger.h
            code/utils.h
            code/utils.h
    )

target_compile_options(dspp PRIVATE -g)

    # Optional: Specify include directories
    target_include_directories(dspp PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/code
    )

    # Optional: Set versioning
    set_target_properties(dspp PROPERTIES
            VERSION 1.0
            SOVERSION 1
    )

target_include_directories(dspp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
)


target_link_libraries(dspp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/hidapi/${CMAKE_GENERATOR_PLATFORM}/${CMAKE_BUILD_TYPE}/hidapi.a
)

add_custom_command(
        TARGET dspp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Running postbuild.sh script"
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/instructions/postbuild.sh ${SOURCE_DIR} ${BUILD_DIR} ${TPTY_DIR}
        COMMENT "Post-build script execution"
)
else()
message("This is for LINUX builds only! Run this on linux")
endif()