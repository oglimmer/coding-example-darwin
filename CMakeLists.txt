cmake_minimum_required(VERSION 3.24)
project(darwin)

set(CMAKE_CXX_STANDARD 23)

add_executable(darwin main.cpp)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(darwin ${SDL2_LIBRARIES})
