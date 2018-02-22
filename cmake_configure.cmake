add_library(rw_interface INTERFACE)
add_library(openrw::interface ALIAS rw_interface)

# target_compile_features(rw_interface INTERFACE cxx_std_14) is not supported by CMake 3.2
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(rw_interface
        INTERFACE
            "-Wall"
            "-Wextra"
            "-Wdouble-promotion"
            "-Wpedantic"
            "-pthread"
        )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(rw_interface
        INTERFACE
            "_SCL_SECURE_NO_WARNINGS"
            "_CRT_SECURE_NO_WARNINGS"
        )
else()
    message(FATAL_ERROR "Unknown compiler ID: '${CMAKE_CXX_COMPILER_ID}'")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(rw_interface
        INTERFACE
            "-Wno-gnu-array-member-paren-init"
        )
endif()

if(MINGW)
    target_compile_options(rw_interface
        INTERFACE
            "-fpermissive"
        )
endif()

target_compile_definitions(rw_interface
    INTERFACE
        "$<$<CONFIG:Debug>:RW_DEBUG=1>"
        "GLM_FORCE_RADIANS"
        "GLM_ENABLE_EXPERIMENTAL"
        "RW_VERBOSE_DEBUG_MESSAGES=$<BOOL:${RW_VERBOSE_DEBUG_MESSAGES}>"
        "RENDER_PROFILER=0"
        "RW_PROFILER=$<BOOL:${ENABLE_PROFILING}>"
    )

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

if(FILESYSTEM_LIBRARY STREQUAL "CXX17")
    target_compile_definitions(rw_interface INTERFACE "RW_FS_LIBRARY=0")
elseif(FILESYSTEM_LIBRARY STREQUAL "CXXTS")
    target_compile_definitions(rw_interface INTERFACE "RW_FS_LIBRARY=1")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(rw_interface INTERFACE "stdc++fs")
    endif()
elseif(FILESYSTEM_LIBRARY STREQUAL "BOOST")
    find_package(Boost COMPONENTS system filesystem REQUIRED)
    target_compile_definitions(rw_interface INTERFACE "RW_FS_LIBRARY=2")
    target_include_directories(rw_interface INTERFACE ${Boost_INCLUDE_DIRS})
    target_link_libraries(rw_interface INTERFACE
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        )
else()
    message(FATAL_ERROR "Illegal FILESYSTEM_LIBRARY option. (was '${FILESYSTEM_LIBRARY}')")
endif()

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
        message("TEST_COVERAGE enabled. Setting CMAKE_BUILD_TYPE to Debug.")
        set(CMAKE_BUILD_TYPE "Debug")
    endif()
    if(NOT BUILD_TESTS)
        message("TEST_COVERAGE enabled. Enabling BUILD_TESTS.")
        set(BUILD_TESTS "ON")
    endif()
    target_compile_options(rw_interface
        INTERFACE
            "-O0"
            "-fprofile-arcs"
            "-ftest-coverage"
        )
    target_link_libraries(rw_interface
        INTERFACE
            gcov
        )
endif()

foreach(SAN ${ENABLE_SANITIZERS})
    if(SAN STREQUAL "address")
        message(STATUS "Address sanitizer enabled.")
        target_compile_options(rw_interface INTERFACE "-fsanitize=address")
        target_link_libraries(rw_interface INTERFACE "-fsanitize=address")
    elseif(SAN STREQUAL "leak")
        message(STATUS "Leak sanitizer enabled.")
        target_compile_options(rw_interface INTERFACE "-fsanitize=leak")
        target_link_libraries(rw_interface INTERFACE "-fsanitize=leak")
    elseif(SAN STREQUAL "thread")
        message(STATUS "Thread sanitizer enabled.")
        target_compile_options(rw_interface INTERFACE "-fsanitize=thread")
        target_link_libraries(rw_interface INTERFACE "-fsanitize=thread")
    elseif(SAN STREQUAL "undefined")
        message(STATUS "Undefined behaviour sanitizer enabled.")
        target_compile_options(rw_interface INTERFACE "-fsanitize=undefined")
        target_link_libraries(rw_interface INTERFACE "-fsanitize=undefined")
    else()
        message(FATAL_ERROR "Illegal sanitizer: ${SAN}")
    endif()
endforeach()

include(CMakeParseArguments)

if(CHECK_IWYU)
    find_package(IncludeWhatYouUse REQUIRED)
endif()

function(openrw_target_apply_options)
    set(IWYU_MAPPING "${PROJECT_SOURCE_DIR}/openrw_iwyu.imp")
    cmake_parse_arguments("OPENRW_APPLY" "" "TARGET" "" ${ARGN})
    if(CHECK_IWYU)
        iwyu_check(TARGET "${OPENRW_APPLY_TARGET}"
            EXTRA_OPTS
                "--mapping_file=${IWYU_MAPPING}"
        )
    endif()

    if(CHECK_CLANGTIDY)
        clang_tidy_check_target(
            TARGET "${OPENRW_APPLY_TARGET}"
            FORMAT_STYLE "file"
            FIX "${CHECK_CLANGTIDY_FIX}"
            CHECK_ALL
        )
    endif()
endfunction()
