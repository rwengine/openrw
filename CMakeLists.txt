cmake_minimum_required(VERSION 2.8)

project(OpenRW)

# Setup options
SET(BUILD_SCRIPT_TOOL TRUE CACHE BOOL "Build script decompiler tool")

# TODO:
# - Visual Studio support
# - Find libraries correctly, e.g. FindBullet

SET(BUILD_OLD_TOOLS FALSE CACHE BOOL "Build old datadump and analyzer tools")

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pthread")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wextra" )
ELSE()
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3" )
ENDIF()

# Make GLM use radians
add_definitions(-DGLM_FORCE_RADIANS)

find_package(Bullet REQUIRED)

IF(BUILD_OLD_TOOLS)
	add_subdirectory(analyzer)
	add_subdirectory(datadump)
ENDIF()

IF(BUILD_SCRIPT_TOOL)
	add_subdirectory(scripttool)
ENDIF()

add_subdirectory(rwgame)
add_subdirectory(rwviewer)
add_subdirectory(rwengine)

add_subdirectory(tests)
