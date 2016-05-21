cmake_minimum_required(VERSION 2.8)

project(OpenRW)

#
# Options
#

# Global Build Configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DRW_DEBUG=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pthread -Wextra -Wpedantic")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Optional components
option(BUILD_TESTS "Build test suite")
option(BUILD_VIEWER "Build GUI data viewer")
option(BUILD_SCRIPT_TOOL "Build script decompiler tool")

# Compile-time Options & Features
option(ENABLE_SCRIPT_DEBUG "Enable verbose script execution")
option(ENABLE_PROFILING "Enable detailed profiling metrics")

#
# Build configuration
#

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_definitions(-DRW_LINUX=1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	add_definitions(-DRW_OSX=1)
else ()
	message(FATAL_ERROR "Unknown platform \"${CMAKE_SYSTEM_NAME}\". please update CMakeLists.txt.")
endif ()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-array-member-paren-init")
endif()

# Make GLM use radians
add_definitions(-DGLM_FORCE_RADIANS)

IF(${ENABLE_PROFILING})
	add_definitions(-DRENDER_PROFILER=0 -DRW_PROFILER=1)
else()
	add_definitions(-DRENDER_PROFILER=0 -DRW_PROFILER=0)
ENDIF()

IF(${ENABLE_SCRIPT_DEBUG})
	add_definitions(-DSCM_DEBUG_INSTRUCTIONS)
ENDIF()

# Find dependancies
IF(APPLE)
	set(OPENRW_PLATFORM_LIBS iconv)
ENDIF()
find_package(OpenGL REQUIRED)
find_package(Bullet REQUIRED)
find_package(SFML 2 COMPONENTS system window audio graphics network REQUIRED)
find_package(MAD REQUIRED)

# External-internal dependencies
add_subdirectory(cmake/external)

#
# Components
#

add_subdirectory(rwlib)
add_subdirectory(rwengine)
add_subdirectory(rwgame)

IF(${BUILD_SCRIPT_TOOL})
	add_subdirectory(scripttool)
ENDIF()
IF(${BUILD_VIEWER})
	add_subdirectory(rwviewer)
ENDIF()
IF(${BUILD_TESTS})
	add_subdirectory(tests)
ENDIF()

