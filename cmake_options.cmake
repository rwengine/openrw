option(RW_VERBOSE_DEBUG_MESSAGES "Print verbose debugging messages" ON)

option(BUILD_TESTS "Build test suite")
option(BUILD_VIEWER "Build GUI data viewer")

option(ENABLE_SCRIPT_DEBUG "Enable verbose script execution")
option(ENABLE_PROFILING "Enable detailed profiling metrics")

option(TESTS_NODATA "Build tests for no-data testing")

set(FAILED_CHECK_ACTION "IGNORE" CACHE STRING "What action to perform on a failed RW_CHECK (in debug mode)")
set_property(CACHE FAILED_CHECK_ACTION PROPERTY STRINGS "IGNORE" "ABORT" "BREAKPOINT")

set(BIN_DIR "bin" CACHE STRING "Prefix subdirectory to put the binaries in.")
set(DOC_DIR "share/doc/openrw" CACHE STRING "Prefix subdirectory to put the documentation in.")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release")
endif()

option(TEST_COVERAGE "Enable coverage analysis (implies CMAKE_BUILD_TYPE=Debug)")
