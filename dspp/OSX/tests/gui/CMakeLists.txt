cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 20)
project(dspp_gui)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DSPP_INCLUDE "../../dspp/build/dspp")
    set(DSPP_LIBRARY_LIB "../../../dspp/build/dspp" )
    set(CMAKE_BUILD_TYPE Debug)
    add_executable(dspp_gui code/main.mm
            3rdparty/include/imgui/imgui.cpp
            3rdparty/include/imgui/imgui_demo.cpp
            3rdparty/include/imgui/imgui_draw.cpp
            3rdparty/include/imgui/imgui_impl_metal.h
            3rdparty/include/imgui/imgui_impl_metal.mm
            3rdparty/include/imgui/imgui_impl_osx.h
            3rdparty/include/imgui/imgui_impl_osx.mm
            3rdparty/include/imgui/imgui_internal.h
            3rdparty/include/imgui/imgui_tables.cpp
            3rdparty/include/imgui/imgui_widgets.cpp
            3rdparty/include/imgui/imstb_rectpack.h
            3rdparty/include/imgui/imstb_textedit.h
            3rdparty/include/imgui/imstb_truetype.h
            code/OPLogger.mm
            code/UIElement.cpp
            code/UIEngine.mm
            code/UIEngine.h
            code/elements/testElement.h)

    set_source_files_properties(
            code/main.mm
            code/OPLogger.mm
            code/UIEngine.h
            code/UIEngine.mm
            3rdparty/include/imgui/imgui_impl_metal.mm
            3rdparty/include/imgui/imgui_impl_osx.mm
            PROPERTIES
            LANGUAGE CXX
            COMPILE_FLAGS "-x objective-c++"
    )


    target_link_libraries(dspp_gui PRIVATE
            ${CMAKE_SOURCE_DIR}/../../dspp/build/dspp/libs/libdspp.a
            ${CMAKE_SOURCE_DIR}/../../dspp/3rdparty/hidapi/x64/debug/hidapi.a
            "-framework CoreFoundation"
            "-framework IOKit"
            "-framework AppKit"
            "-framework Metal"
            "-framework QuartzCore"
            "-framework Foundation"
            "-framework GameController"
            "-framework MetalKit"
    )

    target_include_directories(dspp_gui PRIVATE
            ${DSPP_INCLUDE}/include
            3rdparty/include
            ../../../../common/tests/gui/3rdparty/include
    )

else()
    message("This is for Darwin (MacOS) builds only! Run this on Sonoma or something.")
endif()