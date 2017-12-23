find_program(
    INCLUDE_WHAT_YOU_USE_PROGRAM
    NAMES include_what_you_use iwyu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IncludeWhatYouUse
    REQUIRED_VARS
            INCLUDE_WHAT_YOU_USE_PROGRAM
    )

if(INCLUDEWHATYOUUSE_FOUND)
    include(CMakeParseArguments)

    function(iwyu_check)
        cmake_parse_arguments("IWYU" "" "TARGET" "EXTRA_OPTS" ${ARGN})
        set(IWYU_CMD "${INCLUDE_WHAT_YOU_USE_PROGRAM}")
        foreach(OPT ${IWYU_EXTRA_OPTS})
            list(APPEND IWYU_CMD "-Xiwyu" ${OPT})
        endforeach()
        set_target_properties("${IWYU_TARGET}"
            PROPERTIES
                C_INCLUDE_WHAT_YOU_USE
                    "${IWYU_CMD}"
                CXX_INCLUDE_WHAT_YOU_USE
                    "${IWYU_CMD}"
            )
    endfunction()
endif()
