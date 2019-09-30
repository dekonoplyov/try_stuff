cmake_minimum_required(VERSION 3.0)
project(xtry)

set(CMAKE_CXX_STANDARD 11)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

add_executable(xtry main.cpp)
