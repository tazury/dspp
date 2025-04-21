cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 20)
project(dspp_gui)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

#    set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/include" CACHE PATH "Vulkan include dir")
#    set(Vulkan_LIBRARY "${VULKAN_SDK}/lib/libvulkan.so" CACHE FILEPATH "Vulkan library")
    set(DSPP_INCLUDE "../../dspp/build/dspp")
    set(DSPP_LIBRARY_LIB "../../../dspp/build/dspp" )
    set(CMAKE_BUILD_TYPE Debug)
    add_executable(dspp_gui code/main.cpp
            3rdparty/include/imgui/imgui.cpp
            3rdparty/include/imgui/imgui_demo.cpp
            3rdparty/include/imgui/imgui_draw.cpp
            3rdparty/include/imgui/imgui_impl_sdl3.cpp
            3rdparty/include/imgui/imgui_impl_vulkan.cpp
            3rdparty/include/imgui/imgui_internal.h
            3rdparty/include/imgui/imgui_tables.cpp
            3rdparty/include/imgui/imgui_widgets.cpp
            3rdparty/include/imgui/imstb_rectpack.h
            3rdparty/include/imgui/imstb_textedit.h
            3rdparty/include/imgui/imstb_truetype.h
            code/OPLogger.cpp
            code/UIElement.cpp
            code/UIEngine.cpp
            code/VulkanHelpers.cpp
            code/window.cpp
            code/elements/testElement.h)

    target_link_libraries(dspp_gui PRIVATE
            -Wl,--whole-archive  ${CMAKE_SOURCE_DIR}/../../dspp/build/dspp/libs/libdspp.a
            -Wl,--whole-archive ${CMAKE_SOURCE_DIR}/../../dspp/3rdparty/hidapi/x64/debug/hidapi.a  -Wl,--no-whole-archive
            udev

    )
    message(STATUS "Vulkan Include Dir: ${Vulkan_INCLUDE_DIR}")
    message(STATUS "Vulkan Library: ${Vulkan_LIBRARY}")


    target_include_directories(dspp_gui PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(dspp_gui PRIVATE ${Vulkan_LIBRARY})

#    target_link_libraries(dspp_gui PRIVATE
#            ${VPath}/lib/libvulkan.so
#    )

    include_directories(${VULKAN_SDK}/include)
    link_directories(${VULKAN_SDK}/lib)
    link_libraries(${VULKAN_SDK}/lib/libvulkan.so)
    #target_link_libraries(dspp_gui )

    target_include_directories(dspp_gui PRIVATE
            ${DSPP_INCLUDE}/include
            3rdparty/include
            ../../../../common/tests/gui/3rdparty/include
    )

    find_package(SDL3 REQUIRED CONFIG)
    find_package(Vulkan REQUIRED)
    target_link_libraries(dspp_gui PRIVATE SDL3::SDL3 Vulkan::Vulkan)

else()
    message("This is for LINUX builds only! Run this on linux")
endif()