
add_library(rwcore
    # GL stuff is only here temporarily, hoping to move it back to rwengine
    gl/gl_core_3_3.c
    gl/gl_core_3_3.h
    gl/DrawBuffer.hpp
    gl/DrawBuffer.cpp
    gl/GeometryBuffer.hpp
    gl/GeometryBuffer.cpp
    gl/TextureData.hpp
    gl/TextureData.cpp

    rw/abort.cpp
    rw/casts.hpp
    rw/filesystem.hpp
    rw/forward.hpp
    rw/types.hpp
    rw/debug.hpp

    platform/FileHandle.hpp
    platform/FileIndex.hpp
    platform/FileIndex.cpp

    data/Clump.hpp
    data/Clump.cpp

    fonts/FontMap.cpp
    fonts/FontMap.hpp
    fonts/FontMapGta3.cpp
    fonts/FontMapGta3.hpp
    fonts/GameTexts.cpp
    fonts/GameTexts.hpp
    fonts/Unicode.cpp
    fonts/Unicode.hpp

    loaders/LoaderIMG.hpp
    loaders/LoaderIMG.cpp
    loaders/RWBinaryStream.hpp
    loaders/LoaderDFF.hpp
    loaders/LoaderDFF.cpp
    loaders/LoaderSDT.hpp
    loaders/LoaderSDT.cpp
    loaders/LoaderTXD.hpp
    loaders/LoaderTXD.cpp
    )

if(WIN32)
    target_sources(rwcore
        PRIVATE
            platform/RWWindows.hpp
            platform/RWWindows.cpp
        )
endif()

target_include_directories(rwcore
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

target_link_libraries(rwcore
    PUBLIC
        Boost::boost
        glm::glm
        openrw::interface
    PRIVATE
        OpenGL::GL
    )

openrw_target_apply_options(TARGET rwcore
    FORMAT
    FORMAT_EXCEPT
        gl/gl_core_3_3.c
        gl/gl_core_3_3.h
    )

if(BUILD_SHARED_LIBS)
    install(TARGETS rwcore
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
endif()
