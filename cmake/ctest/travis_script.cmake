cmake_minimum_required(VERSION 3.2.2)

set(TRAVIS "$ENV{TRAVIS}")

if(TRAVIS)
    message("travis_script: Travis detected")
    set(CTEST_SITE "Travis")
    string(SUBSTRING "$ENV{TRAVIS_COMMIT}" 0 8 SHORT_COMMIT)
    set(CTEST_BUILD_NAME "$ENV{TRAVIS_EVENT_TYPE}-$ENV{TRAVIS_REPO_SLUG}-$ENV{TRAVIS_BRANCH}-${SHORT_COMMIT}")
else()
    message("travis_script: Travis NOT detected")
endif()

get_filename_component(SRC_DIR "${SRC_DIR}" ABSOLUTE)
get_filename_component(BIN_DIR "${BIN_DIR}" ABSOLUTE)

set(CTEST_SOURCE_DIRECTORY "${SRC_DIR}")
set(CTEST_BINARY_DIRECTORY "${BIN_DIR}")

set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
ctest_empty_binary_directory("${BIN_DIR}")

ctest_start("Continuous")
ctest_configure(
    OPTIONS
        "-DBUILD_VIEWER=1;-DBUILD_TESTS=1;-DTESTS_NODATA=1"
    RETURN_VALUE
        CONFIGURE_RESULT
    )
ctest_build(
    NUMBER_ERRORS BUILD_NB_ERRORS
    NUMBER_WARNINGS BUILD_NB_WARNINGS
    )

ctest_test(
    RETURN_VALUE TEST_RESULT
    )

file(REMOVE FATAL_ERROR)
if(NOT CONFIGURE_RESULT EQUAL "0")
    message(WARNING "Configure error detected!")
    file(APPEND "FATAL_ERROR" "configure error\n")
endif()

if(NOT BUILD_NB_ERRORS EQUAL "0")
    message(WARNING "Build failure detected!")
    file(APPEND "FATAL_ERROR" "build error\n")
endif()

if(NOT TEST_RESULT EQUAL "0")
    message(WARNING "Test failure detected!")
    file(WRITE "FATAL_ERROR" "test(s) error\n")
endif()

if(TRAVIS)
    ctest_submit(
        RETURN_VALUE SUBMIT_RESULT
        )

    if(NOT SUBMIT_RESULT EQUAL "0")
        message(WARNING "Submit failure detected!")
    endif()
endif()
