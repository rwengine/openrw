# Override CMake's FindOpenGL module:
# - create a OPENGL::GL TARGET

include("${CMAKE_ROOT}/Modules/FindOpenGL.cmake")

if(OPENGL_FOUND AND NOT TARGET OpenGL::GL)
    add_library(OpenGL INTERFACE)
    target_link_libraries(OpenGL
        INTERFACE
            ${OPENGL_LIBRARIES}
        )
    target_include_directories(OpenGL SYSTEM
        INTERFACE
            "${OPENGL_INCLUDE_DIR}"
        )
    add_library(OpenGL::GL ALIAS OpenGL)
endif()
