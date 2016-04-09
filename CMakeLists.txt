cmake_minimum_required(VERSION 2.8)

project(OpenRW)

# Set Global configuration
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DRW_DEBUG 1)
endif()

# Optional components
SET(BUILD_SCRIPT_TOOL TRUE CACHE BOOL "Build script decompiler tool")
SET(BUILD_TESTS TRUE CACHE BOOL "Build test suite")
SET(BUILD_OLD_TOOLS FALSE CACHE BOOL "Build old datadump and analyzer tools")
SET(BUILD_TOOLS FALSE CACHE BOOL "Build editor application")

# Features
SET(ENABLE_PROFILE_RENDERER TRUE CACHE BOOL "Enable higher precision rendering profiler")

# Options
SET(ENABLE_SCRIPT_DEBUG FALSE CACHE BOOL "Enable verbose script execution")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wextra" )
ELSE()
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3" )
ENDIF()

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pthread -Wno-unused-parameter")

IF(APPLE)
	set(OPENRW_PLATFORM_LIBS iconv)
ENDIF()

# Make GLM use radians
add_definitions(-DGLM_FORCE_RADIANS)

IF(${ENABLE_PROFILE_RENDERER})
	add_definitions(-DRENDER_PROFILER=1)
else()
	add_definitions(-DRENDER_PROFILER=0)
ENDIF()

IF(${ENABLE_SCRIPT_DEBUG})
	add_definitions(-DSCM_DEBUG_INSTRUCTIONS)
ENDIF()

find_package(OpenGL REQUIRED)
find_package(Bullet REQUIRED)

IF(BUILD_OLD_TOOLS)
	add_subdirectory(analyzer)
	add_subdirectory(datadump)
ENDIF()

add_subdirectory(rwlib)
add_subdirectory(rwengine)
add_subdirectory(rwgame)

IF(BUILD_SCRIPT_TOOL)
	add_subdirectory(scripttool)
ENDIF()
IF(${BUILD_TOOLS})
	add_subdirectory(rwviewer)
ENDIF()

IF(${BUILD_TESTS})
	add_subdirectory(tests)	
ENDIF()
