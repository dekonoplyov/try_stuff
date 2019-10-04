cmake_minimum_required(VERSION 3.0)
project(win_man LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wcast-align")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(win_man
 main.cpp
 window_manager.cpp)

find_package(glog REQUIRED)
target_link_libraries(win_man glog)
find_package(X11 REQUIRED)
target_link_libraries(win_man X11)
