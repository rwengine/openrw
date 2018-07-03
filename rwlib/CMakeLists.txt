###########################################################
##      RWLIB
###########################################################

SET(RWLIB_SOURCES
    # GL stuff is only here temporarily, hoping to move it back to rwengine
    source/gl/gl_core_3_3.c
    source/gl/gl_core_3_3.h
    source/gl/DrawBuffer.hpp
    source/gl/DrawBuffer.cpp
    source/gl/GeometryBuffer.hpp
    source/gl/GeometryBuffer.cpp
    source/gl/TextureData.hpp
    source/gl/TextureData.cpp

    source/rw/abort.cpp
    source/rw/bit_cast.cpp
    source/rw/filesystem.hpp
    source/rw/forward.hpp
    source/rw/types.hpp
    source/rw/defines.hpp

    source/platform/FileHandle.hpp
    source/platform/FileIndex.hpp
    source/platform/FileIndex.cpp

    source/data/Clump.hpp
    source/data/Clump.cpp

    source/loaders/LoaderIMG.hpp
    source/loaders/LoaderIMG.cpp
    source/loaders/RWBinaryStream.hpp
    source/loaders/LoaderDFF.hpp
    source/loaders/LoaderDFF.cpp
    source/loaders/LoaderSDT.hpp
    source/loaders/LoaderSDT.cpp
    source/loaders/LoaderTXD.hpp
    source/loaders/LoaderTXD.cpp
    )

add_library(rwlib
    ${RWLIB_SOURCES}
    )

target_include_directories(rwlib
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/source"
    )

target_link_libraries(rwlib
    PUBLIC
        rwdep::boost
        rwdep::glm
        openrw::interface
    PRIVATE
        OpenGL::GL
    )

set_target_properties(rwlib
        PROPERTIES
            OUTPUT_NAME rw
    )

openrw_target_apply_options(TARGET rwlib)

if(BUILD_SHARED_LIBS)
    install(TARGETS rwlib
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
endif()
