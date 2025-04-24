cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 20)
project(dspp_project)

add_subdirectory(dspp)
add_subdirectory(tests/cmdline)
add_subdirectory(tests/gui)