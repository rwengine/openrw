# - Try to find clang-tidy executable and create clang_tidy_check_target function to check targets
# Once done this will define
#
# CLANG_TIDY_FOUND - system has clang-tidy
# CLANG_TIDY_BIN - path of clang-tidy
# CLANG_TIDY_VERSION - version of clang-tidy
#
# clang_tidy_check_target function:
# usage:
# clang_tidy_check_target(
#     TARGET <target>                   # Target to attach clang-tidy to
#     [FORMAT_STYLE <format_style>]     # Format of clang-tidy output
#     [FIX <fix_bool>]                  # Apply the clang-tidy fixes to the sources
#     [CHECK_ALL]                       # run all clang-tidy checks
#     )
# Copyright (c) 2018 OpenRW project
#
# Redistribution and use is allowed according to the terms of the New BSD license.
#

find_program(CLANG_TIDY_BIN
    clang-tidy
)

if(CLANG_TIDY_BIN)
    set(CLANG_TIDY_FOUND 1)
else()
    set(CLANG_TIDY_FOUND 0)
endif()

if(CLANG_TIDY_BIN)
    execute_process(
        COMMAND "${CLANG_TIDY_BIN}" --version
        OUTPUT_VARIABLE _CLANG_TIDY_VERSION_OUTPUT
        )
    string(REGEX MATCH "version ([0-9a-zA-Z\.]+)" _CLANG_TIDY_VERSION "${_CLANG_TIDY_VERSION_OUTPUT}")
    if(_CLANG_TIDY_VERSION)
        set(CLANG_TIDY_VERSION "${CMAKE_MATCH_1}")
    else()
        set(CLANG_TIDY_VERSION "unknown")
    endif()

    execute_process(
        COMMAND "${CLANG_TIDY_BIN}" --help
        OUTPUT_VARIABLE _CLANG_TIDY_HELP_OUTPUT
        )
    string(FIND "${_CLANG_TIDY_HELP_OUTPUT}" "-format-style" _CLANG_TIDY_FORMAT_STYLE_POS)
    if(_CLANG_TIDY_FORMAT_STYLE_POS LESS 0)
        set(CLANG_TIDY_FORMAT_STYLE_OPTION "-style=")
    else()
        set(CLANG_TIDY_FORMAT_STYLE_OPTION "-format-style=")
    endif()

    include(CMakeParseArguments)
    function(clang_tidy_check_target)
        cmake_parse_arguments(CTCT
            "CHECK_ALL" #options
            "TARGET;FORMAT_STYLE;FIX" #one_value_keywords
            "" #multi_value_keywords
            ${ARGN}
            )

        set(CLANGTIDY_ARGS "${CLANG_TIDY_BIN}")
        if(CTCT_CHECK_ALL)
            list(APPEND CLANGTIDY_ARGS "-checks=*")
        endif()
        if(CTCT_FORMAT_STYLE)
            list(APPEND CLANGTIDY_ARGS "${CLANG_TIDY_FORMAT_STYLE_OPTION}${CTCT_FORMAT_STYLE}")
        endif()
        if(CTCT_FIX)
            list(APPEND CLANGTIDY_ARGS "-fix")
        endif()

        set_target_properties("${CTCT_TARGET}"
            PROPERTIES
                C_CLANG_TIDY "${CLANGTIDY_ARGS}"
                CXX_CLANG_TIDY "${CLANGTIDY_ARGS}"
        )
    endfunction()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangTidy
    REQUIRED_VARS CLANG_TIDY_BIN
    VERSION_VAR CLANG_TIDY_VERSION
    )
mark_as_advanced(CLANG_TIDY_BIN)
