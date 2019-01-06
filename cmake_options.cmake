option(RW_VERBOSE_DEBUG_MESSAGES "Print verbose debugging messages" ON)

option(BUILD_TOOLS "Build tools")
option(BUILD_TESTS "Build test suite")
option(BUILD_VIEWER "Build GUI data viewer")

option(ENABLE_SCRIPT_DEBUG "Enable verbose script execution")
option(ENABLE_PROFILING "Enable detailed profiling metrics")

option(TESTS_NODATA "Build tests for no-data testing")

set(FAILED_CHECK_ACTION "IGNORE" CACHE STRING "What action to perform on a failed RW_CHECK (in debug mode)")
set_property(CACHE FAILED_CHECK_ACTION PROPERTY STRINGS "IGNORE" "ABORT" "BREAKPOINT")

set(FILESYSTEM_LIBRARY "BOOST" CACHE STRING "Which filesystem library to use")
set_property(CACHE FILESYSTEM_LIBRARY PROPERTY STRINGS "CXX17" "CXXTS" "BOOST")

set(CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo;MinSizeRel" CACHE INTERNAL "Build types supported by this project.")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}")
endif()
if(MSVC)
    option(MSVC_NO_DEBUG_RUNTIME "Don't use the debug runtime")
endif()

option(CHECK_IWYU "Enable IncludeWhatYouUse (Analyze #includes in C and C++ source files)")
option(CHECK_CLANGTIDY "Enable clang-tidy (A clang-based C++ linter tool)")
option(CHECK_CLANGTIDY_FIX "Apply fixes from clang-tidy (!!!RUN ON CLEAN GIT TREE!!!)")

set(ENABLE_SANITIZERS "" CACHE STRING "Enable selected sanitizer.")

set(CODECOV_NAME "" CACHE STRING "Name of report in codecov.io ui.")
option(TEST_COVERAGE "Enable coverage analysis (implies CMAKE_BUILD_TYPE=Debug)")
option(SEPARATE_TEST_SUITES "Add each test suite as separate test to CTest")

option(BOOST_STATIC "Link against static Boost libraries")

option(USE_CONAN "Use Conan as package manager")
