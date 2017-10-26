cmake_minimum_required(VERSION 2.8)

project(OpenRW)

#
# Options
#

set(RW_VERBOSE_DEBUG_MESSAGES 1 CACHE BOOL "Print verbose debugging messages")

# Optional components
option(BUILD_TESTS "Build test suite")
option(BUILD_VIEWER "Build GUI data viewer")

# Compile-time Options & Features
option(ENABLE_SCRIPT_DEBUG "Enable verbose script execution")
option(ENABLE_PROFILING "Enable detailed profiling metrics")
option(TESTS_NODATA "Build tests for no-data testing")

#
# Global Build Configuration
#
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(FAILED_CHECK_ACTION "IGNORE" CACHE STRING "What action to perform on a failed RW_CHECK (IGNORE, ABORT or BREAKPOINT) (only in debug mode)")

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
  add_definitions(-DRW_DEBUG=1)
endif()

#
# Build configuration
#
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_definitions(-DRW_LINUX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	add_definitions(-DRW_OSX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	add_definitions(-DRW_FREEBSD)
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
	add_definitions(-DRW_NETBSD)
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	add_definitions(-DRW_OPENBSD)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_definitions(-DRW_WINDOWS)
else ()
	message(FATAL_ERROR "Unknown platform \"${CMAKE_SYSTEM_NAME}\". please update CMakeLists.txt.")
endif ()

if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -Wextra -Wpedantic -Wdouble-promotion")
endif()

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-array-member-paren-init")
endif()

if(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

IF(${ENABLE_PROFILING})
	add_definitions(-DRENDER_PROFILER=0 -DRW_PROFILER=1)
else()
	add_definitions(-DRENDER_PROFILER=0 -DRW_PROFILER=0)
ENDIF()

IF(${ENABLE_SCRIPT_DEBUG})
	add_definitions(-DRW_SCRIPT_DEBUG)
ENDIF()

if(FAILED_CHECK_ACTION STREQUAL "IGNORE")
    add_definitions(-DRW_FAILED_CHECK_ACTION=0)
elseif(FAILED_CHECK_ACTION STREQUAL "ABORT")
    add_definitions(-DRW_FAILED_CHECK_ACTION=1)
elseif(FAILED_CHECK_ACTION STREQUAL "BREAKPOINT")
    add_definitions(-DRW_FAILED_CHECK_ACTION=2)
else()
    message(FATAL_ERROR "Illegal FAILED_CHECK_ACTION option.")
endif()

if(${RW_VERBOSE_DEBUG_MESSAGES})
	add_definitions(-DRW_VERBOSE_DEBUG_MESSAGES=1)
else()
	add_definitions(-DRW_VERBOSE_DEBUG_MESSAGES=0)
endif()

if(NOT DEFINED BIN_DIR)
        set(BIN_DIR "bin" CACHE PATH "BIN_DIR")
endif()

if(NOT DEFINED DOC_DIR)
        set(DOC_DIR "share/doc/openrw" CACHE PATH "DOC_DIR")
endif()

# Find dependancies
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Bullet REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(
	${SDL2_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${BULLET_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
)

# Imported dependencies
add_subdirectory(external)

# Include git hash in source
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

#
# Components
#

add_subdirectory(rwlib)
add_subdirectory(rwengine)
add_subdirectory(rwgame)

IF(${BUILD_VIEWER})
	add_subdirectory(rwviewer)
ENDIF()
IF(${BUILD_TESTS})
	add_subdirectory(tests)
ENDIF()

#
# Finally
#

# Copy License file to install directory
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
	install(FILES COPYING
			DESTINATION "${DOC_DIR}"
			)
endif()
# And copy to build directory for CI
file(COPY COPYING
	 DESTINATION .
	 )
