add_library(rw_interface INTERFACE)
add_library(openrw::interface ALIAS rw_interface)

add_library(rw_checks INTERFACE)
add_library(openrw::checks ALIAS rw_checks)
target_link_libraries(rw_interface INTERFACE rw_checks)

# target_compile_features(rw_interface INTERFACE cxx_std_14) is not supported by CMake 3.2
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(rw_interface
        INTERFACE
            "-Wall"
            "-Wextra"
            "-Wdouble-promotion"
            "-Wpedantic"
            "-Wmissing-braces"
            "$<IF:$<COMPILE_LANGUAGE:CXX>,-Wold-style-cast,>"
        )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(rw_checks
        INTERFACE
            "_SCL_SECURE_NO_WARNINGS"
            "_CRT_SECURE_NO_WARNINGS"
        )
    target_compile_options(rw_interface
        INTERFACE
            "/MP"
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

if(WIN32)
    # Required minimum version of Windows = Vista
    set(RW_NTDDI_VISTA 0x06000000)
    set(RW_WINVER 0x0600)
    target_compile_definitions(rw_interface
        INTERFACE
            "NTDDI_VERSION=${RW_NTDDI_VISTA}"
            "WINVER=${RW_WINVER}"
            "_WIN32_WINNT=${RW_WINVER}"
        )
endif()

target_compile_definitions(rw_interface
    INTERFACE
        "$<$<CONFIG:Debug>:RW_DEBUG>"
        "GLM_FORCE_PURE"
        "GLM_FORCE_RADIANS"
        "GLM_ENABLE_EXPERIMENTAL"
        "$<$<BOOL:${RW_VERBOSE_DEBUG_MESSAGES}>:RW_VERBOSE_DEBUG_MESSAGES>"
        "$<$<BOOL:${ENABLE_PROFILING}>:RW_PROFILER>"
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

target_compile_definitions(rw_interface
    INTERFACE
        BOOST_ALL_NO_LIB
    )

if(NOT BOOST_STATIC)
    target_compile_definitions(rw_interface
        INTERFACE
            BOOST_ALL_DYN_LINK
    	)
endif()

if(USE_CONAN)
    if(CONAN_SETTINGS_COMPILER_LIBCXX STREQUAL "libstdc++11")
        target_compile_definitions(rw_interface INTERFACE _GLIBCXX_USE_CXX11_ABI=1)
    elseif(CONAN_SETTINGS_COMPILER_LIBCXX STREQUAL "libstdc++")
        target_compile_definitions(rw_interface INTERFACE _GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()

if(FILESYSTEM_LIBRARY STREQUAL "CXX17")
    set(CMAKE_CXX_STANDARD 17)
    target_compile_definitions(rw_interface INTERFACE "RW_FS_LIBRARY=0")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_libraries(rw_interface INTERFACE "stdc++fs")
    endif()
elseif(FILESYSTEM_LIBRARY STREQUAL "CXXTS")
    target_compile_definitions(rw_interface INTERFACE "RW_FS_LIBRARY=1")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_libraries(rw_interface INTERFACE "stdc++fs")
    endif()
elseif(FILESYSTEM_LIBRARY STREQUAL "BOOST")
    target_compile_definitions(rw_interface INTERFACE "RW_FS_LIBRARY=2")
    target_link_libraries(rw_interface INTERFACE
        Boost::filesystem
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
    target_compile_options(rw_checks
        INTERFACE
            "-O0"
            "-fprofile-arcs"
            "-ftest-coverage"
        )
    target_link_libraries(rw_checks
        INTERFACE
            gcov
        )
endif()

foreach(SAN ${ENABLE_SANITIZERS})
    if(SAN STREQUAL "address")
        message(STATUS "Address sanitizer enabled.")
        target_compile_options(rw_checks INTERFACE "-fsanitize=address")
        target_link_libraries(rw_checks INTERFACE "-fsanitize=address")
    elseif(SAN STREQUAL "leak")
        message(STATUS "Leak sanitizer enabled.")
        target_compile_options(rw_checks INTERFACE "-fsanitize=leak")
        target_link_libraries(rw_checks INTERFACE "-fsanitize=leak")
    elseif(SAN STREQUAL "thread")
        message(STATUS "Thread sanitizer enabled.")
        target_compile_options(rw_checks INTERFACE "-fsanitize=thread")
        target_link_libraries(rw_checks INTERFACE "-fsanitize=thread")
    elseif(SAN STREQUAL "undefined")
        message(STATUS "Undefined behaviour sanitizer enabled.")
        target_compile_options(rw_checks INTERFACE "-fsanitize=undefined")
        target_link_libraries(rw_checks INTERFACE "-fsanitize=undefined")
    else()
        message(FATAL_ERROR "Illegal sanitizer: ${SAN}")
    endif()
endforeach()

function(openrw_target_apply_options)
    set(IWYU_MAPPING "${PROJECT_SOURCE_DIR}/openrw_iwyu.imp")
    cmake_parse_arguments("ORW" "INSTALL;INSTALL_PDB" "TARGET" "" ${ARGN})
    if(CHECK_IWYU)
        iwyu_check(TARGET "${ORW_TARGET}"
            EXTRA_OPTS
                "--mapping_file=${IWYU_MAPPING}"
        )
    endif()

    if(CHECK_CLANGTIDY)
        clang_tidy_check_target(
            TARGET "${ORW_TARGET}"
            FORMAT_STYLE "file"
            FIX "${CHECK_CLANGTIDY_FIX}"
            CHECK_ALL
        )
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set_property(
            TARGET "${ORW_TARGET}"
            APPEND
            PROPERTY STATIC_LIBRARY_FLAGS "-no_warning_for_no_symbols"
            )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_property(
            TARGET "${ORW_TARGET}"
            APPEND
            PROPERTY LINK_FLAGS "/ignore:4099"
            )
    endif()

    if(ORW_INSTALL)
        install(
            TARGETS "${ORW_TARGET}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            )
    endif()
    if(ORW_INSTALL_PDB)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            install(FILES "$<$<OR:$<CONFIG:DEBUG>,$<CONFIG:RELWITHDEBINFO>>:$<TARGET_PDB_FILE:${ORW_TARGET}>>"
                DESTINATION "${CMAKE_INSTALL_BINDIR}"
                )
        endif()
    endif()

endfunction()
