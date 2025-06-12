cmake_minimum_required(VERSION 3.8...3.28)

project(OpenRW)

include(GNUInstallDirs)

# Read the configuration arguments
include("${PROJECT_SOURCE_DIR}/cmake_options.cmake")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# Include git hash in source
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
message(STATUS "Building ${CMAKE_PROJECT_NAME} GIT SHA1: ${GIT_SHA1}")

include(WrapTargets)
if(USE_CONAN)
    if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake")
        message(STATUS "Using conan 'cmake_multi' generator")
        include("${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake")
    else()
        message(STATUS "Using conan 'cmake' generator")
        include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    endif()
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

    rwdep_wrap_conan_targets()
else()
    if(POLICY CMP0167)
	cmake_policy(SET CMP0167 NEW)
    endif()
    find_package(Boost REQUIRED)
    find_package(Boost COMPONENTS program_options system REQUIRED)
    if(BUILD_TESTS)
        find_package(Boost COMPONENTS unit_test_framework REQUIRED)
    endif()
    if(BUILD_TOOLS)
        find_package(Freetype REQUIRED)
    endif()

    # Do not link to SDL2main library
    set(SDL2_BUILDING_LIBRARY TRUE)

    find_package(OpenAL REQUIRED)
    find_package(Bullet REQUIRED)
    find_package(GLM REQUIRED)
    find_package(FFmpeg REQUIRED)
    find_package(SDL2 REQUIRED)

    rwdep_wrap_find_packages()
endif()

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

if(CHECK_CLANGTIDY)
    find_package(ClangTidy REQUIRED)
endif()

if(CHECK_IWYU)
    find_package(IncludeWhatYouUse REQUIRED)
endif()

# Create a rw_interface TARGET that holds all compiler options
include("${PROJECT_SOURCE_DIR}/cmake_configure.cmake")

add_subdirectory(external)

add_subdirectory(rwcore)
add_subdirectory(rwengine)
add_subdirectory(rwgame)

if(BUILD_VIEWER)
    add_subdirectory(rwviewer)
endif()
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(BUILD_TOOLS)
    add_subdirectory(rwtools)
endif()

# Copy the license to the install directory
install(FILES COPYING
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    )

# Copy the license to the build directory (for CI)
file(COPY COPYING
    DESTINATION "${PROJECT_BINARY_DIR}"
    )

include(CMakeCPack.cmake)

if(TEST_COVERAGE)
    coverage_collect()
endif()
