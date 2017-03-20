cmake_minimum_required(VERSION 3.3)
project(project2Server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(HEADER_FILES $main.h)
set(SOURCE_FILES main.cpp)
add_executable(project2Server ${SOURCE_FILES})