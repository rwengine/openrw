cmake_minimum_required(VERSION 2.8)

project(OpenRW)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall" )

# Make GLM use radians
add_definitions(-DGLM_FORCE_RADIANS)

add_subdirectory(datadump)
add_subdirectory(rwgame)
add_subdirectory(rwviewer)
add_subdirectory(rwengine)

add_subdirectory(analyzer)

add_subdirectory(tests)
