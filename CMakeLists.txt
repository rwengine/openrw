cmake_minimum_required(VERSION 2.8)

project(gtfw)

SET(CMAKE_CXX_FLAGS "-g -std=c++11")

add_subdirectory(datadump)
add_subdirectory(viewer)

add_subdirectory(framework2)
add_subdirectory(analyzer)

add_subdirectory(tests)
