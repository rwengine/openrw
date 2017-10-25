cmake_minimum_required(VERSION 3.2.2)

project(OpenRW)

#
# Options
#

option(RW_VERBOSE_DEBUG_MESSAGES "Print verbose debugging messages" ON)

# Optional components
option(BUILD_TESTS "Build test suite")
option(BUILD_VIEWER "Build GUI data viewer")

# Compile-time Options & Features
option(ENABLE_SCRIPT_DEBUG "Enable verbose script execution")
option(ENABLE_PROFILING "Enable detailed profiling metrics")
option(TESTS_NODATA "Build tests for no-data testing")

set(FAILED_CHECK_ACTION "IGNORE" CACHE STRING "What action to perform on a failed RW_CHECK (in debug mode)")
set_property(CACHE FAILED_CHECK_ACTION PROPERTY STRINGS "IGNORE" "ABORT" "BREAKPOINT")

#
# Build configuration
#
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

add_library(rw_interface INTERFACE)
add_library(openrw::interface ALIAS rw_interface)

# target_compile_features(rw_interface INTERFACE cxx_std_14) is not supported by CMake 3.2
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_compile_options(rw_interface
    INTERFACE
        "-Wall"
        "-Wextra"
        "-Wdouble-promotion"
        "-Wpedantic"
        "-pthread"
    )
target_compile_definitions(rw_interface
    INTERFACE
        "$<$<CONFIG:Debug>:RW_DEBUG=1>"
        "GLM_FORCE_RADIANS"
        "RW_VERBOSE_DEBUG_MESSAGES=$<BOOL:${RW_VERBOSE_DEBUG_MESSAGES}>"
    )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(rw_interface INTERFACE "RW_LINUX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(rw_interface INTERFACE "RW_OSX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_compile_definitions(rw_interface INTERFACE "RW_FREEBSD")
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    target_compile_definitions(rw_interface INTERFACE "RW_NETBSD")
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    target_compile_definitions(rw_interface INTERFACE "RW_OPENBSD")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(rw_interface INTERFACE "RW_WINDOWS")
else()
    message(FATAL_ERROR "Unknown platform \"${CMAKE_SYSTEM_NAME}\". please update CMakeLists.txt.")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    target_compile_options(rw_interface INTERFACE "-Wno-gnu-array-member-paren-init")
endif()

if(MINGW)
    target_compile_options(rw_interface INTERFACE "-fpermissive")
endif()

target_compile_definitions(rw_interface
    INTERFACE
        "RENDER_PROFILER=0"
        "RW_PROFILER=$<BOOL:${ENABLE_PROFILING}>"
    )

if(ENABLE_SCRIPT_DEBUG)
    target_compile_definitions(rw_interface
        INTERFACE
            "RW_SCRIPT_DEBUG"
    )
endif()

if(FAILED_CHECK_ACTION STREQUAL "IGNORE")
    target_compile_definitions(rw_interface INTERFACE "RW_FAILED_CHECK_ACTION=0")
elseif(FAILED_CHECK_ACTION STREQUAL "ABORT")
    target_compile_definitions(rw_interface INTERFACE "RW_FAILED_CHECK_ACTION=1")
elseif(FAILED_CHECK_ACTION STREQUAL "BREAKPOINT")
    target_compile_definitions(rw_interface INTERFACE "RW_FAILED_CHECK_ACTION=2")
else()
    message(FATAL_ERROR "Illegal FAILED_CHECK_ACTION option.")
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
find_package(GLM REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(SDL2 REQUIRED)

# External-internal dependencies
add_subdirectory(cmake/external)

# Include git hash in source
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

#
# Components
#

add_subdirectory(rwlib)
add_subdirectory(rwengine)
add_subdirectory(rwgame)

if(BUILD_VIEWER)
    add_subdirectory(rwviewer)
endif()
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

#
# Finally
#

# Copy License file to install directory
if(CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD")
    install(FILES COPYING
        DESTINATION "${DOC_DIR}"
    )
endif()
# And copy to build directory for CI
file(COPY COPYING
    DESTINATION .
    )
