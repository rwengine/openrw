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
    message(FATAL_ERROR "Illegal FAILED_CHECK_ACTION option. (was '${FAILED_CHECK_ACTION}')")
endif()

if(TEST_COVERAGE)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Test Coverage enabled. Setting CMAKE_BUILD_TYPE to Debug.")
        set(CMAKE_BUILD_TYPE "Debug")
    endif()
    if(NOT BUILD_TESTS)
        message("TestCoverage enabled. Enabling BUILD_TESTS.")
        set(BUILD_TESTS "ON")
    endif()
    target_compile_options(rw_interface
        INTERFACE
            "-O0"
            "-fprofile-arcs"
            "-ftest-coverage"
        )
    set_target_properties(rw_interface
        PROPERTIES
            INTERFACE_LINK_LIBRARIES
                gcov
    )
endif()

include(CMakeParseArguments)

if(CHECK_INCLUDES)
    find_package(IncludeWhatYouUse REQUIRED)
endif()

function(openrw_target_apply_options)
    cmake_parse_arguments("OPENRW_APPLY" "" "TARGET" "" ${ARGN})
    if(CHECK_INCLUDES)
        iwyu_check(TARGET "${OPENRW_APPLY_TARGET}"
            EXTRA_OPTS
        )
    endif()
endfunction()
