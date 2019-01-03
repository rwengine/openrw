add_library(rw_interface INTERFACE)
add_library(openrw::interface ALIAS rw_interface)

add_library(rw_checks INTERFACE)
add_library(openrw::checks ALIAS rw_checks)
target_link_libraries(rw_interface INTERFACE rw_checks)
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
    if(MSVC_NO_DEBUG_RUNTIME)
        foreach(LANG C CXX)
            foreach(CONFIGURATION_TYPE ${CMAKE_CONFIGURATION_TYPES} "")
                string(TOUPPER "${CONFIGURATION_TYPE}" CONFIGURATION_TYPE)
                set(FLAGS_VAR "CMAKE_${LANG}_FLAGS")
                if(CONFIGURATION_TYPE)
                    set(FLAGS_VAR "${FLAGS_VAR}_${CONFIGURATION_TYPE}")
                endif()
                string(REPLACE "/MDd" "/MD" "${FLAGS_VAR}" "${${FLAGS_VAR}}")
                string(REPLACE "/MTd" "/MT" "${FLAGS_VAR}" "${${FLAGS_VAR}}")
                set("${FLAGS_VAR}" "${${FLAGS_VAR}}" CACHE STRING "${LANG} flags for ${CONFIGURATION_TYPE} configuration type")
            endforeach()
        endforeach()
    endif()
    target_compile_definitions(rw_checks
        INTERFACE
            "_SCL_SECURE_NO_WARNINGS"
            "_CRT_SECURE_NO_WARNINGS"
        )
    target_compile_options(rw_interface
        INTERFACE
            "/MP"
            "/Zc:__cplusplus"
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

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(rw_interface INTERFACE "stdc++fs")
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
    include(CodeCoverage)
    codecoverage_enable("${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/codecoverage")

    string(REGEX MATCH "[^/]*/[^/]*/([^/]*)" _RE_REFSPEC "${GIT_REFSPEC}")
    if(_RE_REFSPEC)
        set(GIT_BRANCH "${CMAKE_MATCH_1}")
        message(STATUS "Git branch detected from .git directory: ${GIT_BRANCH}")
    elseif(DEFINED ENV{TRAVIS_BRANCH})
        set(GIT_BRANCH "$ENV{TRAVIS_BRANCH}")
        message(STATUS "Git branch detected from TRAVIS_BRANCH environment variable: ${GIT_BRANCH}")
    else()
        message("Git branch unknown: uploading coverage might nog work")
    endif()
    add_coverage_upload_target_codecov_io(
        GITSHA1 "${GIT_SHA1}"
        GITBRANCH "${GIT_BRANCH}"
        NAME "${CODECOV_NAME}"
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
    cmake_parse_arguments("ORW" "CORE;INSTALL;INSTALL_PDB;COVERAGE" "TARGET" "COVERAGE_EXCEPT" ${ARGN})

    if(TEST_COVERAGE AND ORW_COVERAGE)
        coverage_add_target("${ORW_TARGET}" EXCEPT ${ORW_COVERAGE_EXCEPT})
    endif()

    if(ORW_CORE)
        if(CHECK_IWYU)
            iwyu_check(TARGET "${ORW_TARGET}"
                EXTRA_OPTS
                    "--mapping_file=${IWYU_MAPPING}"
            )
        endif()

        set_target_properties("${ORW_TARGET}"
            PROPERTIES
                CXX_EXTENSIONS OFF
                CXX_STANDARD 17
                C_EXTENSIONS OFF
                C_STANDARD 11
            )

        if(CHECK_CLANGTIDY)
            clang_tidy_check_target(
                TARGET "${ORW_TARGET}"
                FORMAT_STYLE "file"
                FIX "${CHECK_CLANGTIDY_FIX}"
                CHECK_ALL
            )
        endif()
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
