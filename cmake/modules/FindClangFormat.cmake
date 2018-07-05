# - Try to find clang-format executable
# - Create clang_format_sources and clang_format_target functions
#
# Once done this will define
#
# CLANG_FORMAT_FOUND - system has clang-format
# CLANG_FORMAT_BIN - path of clang-format
# CLANG_FORMAT_VERSION - version of clang-format
#
# clang_format_sources function:
# usage:
# clang_format_sources(
#     [STYLE <style>]                   # coding style (default = file)
#     [NAME <style>]                    # name for the clang-format target
#     <source> [<source> ...]           # Sources to process
#     )
#
# clang_format_target function:
# usage:
# clang_format_target(
#     TARGET <target>                   # target to process
#     [STYLE <style>]                   # coding style (default = file)
#     )
#
# Copyright (c) 2018 OpenRW project
#
# Redistribution and use is allowed according to the terms of the New BSD license.
#

find_program(CLANG_FORMAT_BIN
    clang-format
)

if(CLANG_FORMAT_BIN)
    set(CLANG_FORMAT_FOUND 1)
else()
    set(CLANG_FORMAT_FOUND 0)
endif()

if(CLANG_FORMAT_BIN)
    find_package(Git REQUIRED)

    add_custom_target(clang-format)

    execute_process(
        COMMAND "${CLANG_FORMAT_BIN}" --version
        OUTPUT_VARIABLE _CLANG_FORMAT_VERSION_OUTPUT
        )
    string(REGEX MATCH "version ([0-9a-zA-Z\.]+)" _CLANG_FORMAT_VERSION "${_CLANG_FORMAT_VERSION_OUTPUT}")
    if(_CLANG_FORMAT_VERSION)
        set(CLANG_FORMAT_VERSION "${CMAKE_MATCH_1}")
    else()
        set(CLANG_FORMAT_VERSION "unknown")
    endif()

    function(clang_format_sources)
        cmake_parse_arguments(CFCF
            "" #options
            "NAME;STYLE" #one_value_keywords
            "" #multi_value_keywords
            ${ARGN}
            )
        if(NOT CFCF_STYLE)
            set(CFCF_STYLE "file")
        endif()
        if(NOT CFCF_NAME)
            string(RANDOM LENGTH 16 CF_RANDOM)
            set(CFCF_NAME "CLANG_FORMAT_${CF_RANDOM}")
        endif()
        set(SOURCES "${CFCF_UNPARSED_ARGUMENTS}")
        string(RANDOM LENGTH 16 CF_TARGET_NAME)
        add_custom_target("${CFCF_NAME}"
            COMMAND "${CLANG_FORMAT_BIN}" "-i" "-style=${CFCF_STYLE}" ${SOURCES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            COMMENT "Running clang-format on sources..."
            )
        add_dependencies(clang-format "${CFCF_NAME}")
    endfunction()

    function(clang_format_target)
        cmake_parse_arguments(CFCF
            "" #options
            "STYLE;TARGET" #one_value_keywords
            "EXCEPT" #multi_value_keywords
            ${ARGN}
            )
        if(NOT CFCF_TARGET)
            message(FATAL_ERROR "clang_format_target needs a TARGET")
        endif()
        if(NOT CFCF_STYLE)
            set(CFCF_STYLE "file")
        endif()
        get_target_property(SOURCES "${CFCF_TARGET}" SOURCES)
        if(CFCF_EXCEPT)
            list(REMOVE_ITEM SOURCES ${CFCF_EXCEPT})
        endif()
        clang_format_sources(
            NAME "CLANG_FORMAT_${CFCF_TARGET}"
            STYLE "${CFCF_STYLE}"
            ${SOURCES}
            )
    endfunction()

    add_custom_target(clang-format-check
        "${GIT_EXECUTABLE}" diff --exit-code
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        )
    add_dependencies(clang-format-check clang-format)

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangFormat
    REQUIRED_VARS CLANG_FORMAT_BIN
    VERSION_VAR CLANG_FORMAT_VERSION
    )
mark_as_advanced(CLANG_FORMAT_BIN)
