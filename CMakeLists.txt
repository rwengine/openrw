cmake_minimum_required(VERSION 3.2.2)

project(OpenRW)

# Read the configuration arguments
include("${PROJECT_SOURCE_DIR}/cmake_options.cmake")

# Create a rw_interface TARGET that holds all compiler options
include("${PROJECT_SOURCE_DIR}/cmake_configure.cmake")

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Bullet REQUIRED)
find_package(GLM REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(SDL2 REQUIRED)

# Include git hash in source
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_subdirectory(rwlib)
add_subdirectory(rwengine)
add_subdirectory(rwgame)

if(BUILD_VIEWER)
    add_subdirectory(rwviewer)
endif()
if(BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()

# Copy the license to the install directory
if(CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD")
    install(FILES COPYING
        DESTINATION "${DOC_DIR}"
        )
endif()

# Copy the license to the build directory (for CI)
file(COPY COPYING
    DESTINATION "${PROJECT_BINARY_DIR}"
    )
