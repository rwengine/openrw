cmake_minimum_required(VERSION 2.8)

project(OpenRW)

# TODO:
# - Visual Studio support
# - Find libraries correctly, e.g. FindBullet

SET(BUILD_OLD_TOOLS FALSE CACHE BOOL "Build old datadump and analyzer tools")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall -Wextra" )
ELSE()
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -Wall" )
ENDIF()

# Make GLM use radians
add_definitions(-DGLM_FORCE_RADIANS)

find_package(Bullet REQUIRED)

IF(BUILD_OLD_TOOLS)
	add_subdirectory(analyzer)
	add_subdirectory(datadump)
ENDIF()

add_subdirectory(rwgame)
add_subdirectory(rwviewer)
add_subdirectory(rwengine)

add_subdirectory(tests)
