function(rwdep_wrap_find_packages)
    if(BULLET_FOUND AND NOT TARGET bullet::bullet)
        add_library(bullet::bullet INTERFACE IMPORTED)
        set_property(TARGET bullet::bullet
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BULLET_INCLUDE_DIR})
        set_property(TARGET bullet::bullet
            PROPERTY INTERFACE_LINK_LIBRARIES ${BULLET_LIBRARIES})
    endif()

    if(OPENAL_FOUND AND NOT TARGET OpenAL::OpenAL)
        add_library(OpenAL::OpenAL INTERFACE IMPORTED)
        set_property(TARGET OpenAL::OpenAL
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR})
        set_property(TARGET OpenAL::OpenAL
            PROPERTY INTERFACE_LINK_LIBRARIES ${OPENAL_LIBRARY})
    endif()
endfunction()

function(rwdep_wrap_conan_target TARGET CONAN_NAME)
    add_library("${TARGET}" INTERFACE IMPORTED)
    set_property(TARGET "${TARGET}"
        PROPERTY INTERFACE_LINK_LIBRARIES "CONAN_PKG::${CONAN_NAME}")
endfunction()

function(rwdep_wrap_conan_targets)
    rwdep_wrap_conan_target(OpenAL::OpenAL openal)
    rwdep_wrap_conan_target(bullet::bullet bullet3)
    rwdep_wrap_conan_target(glm::glm glm)
    rwdep_wrap_conan_target(ffmpeg::ffmpeg ffmpeg)
    rwdep_wrap_conan_target(SDL2::SDL2 sdl2)
    if(BUILD_TOOLS)
        rwdep_wrap_conan_target(Freetype::Freetype freetype)
    endif()

    rwdep_wrap_conan_target(Boost::boost boost)
    rwdep_wrap_conan_target(Boost::program_options boost)
    rwdep_wrap_conan_target(Boost::system boost)
    rwdep_wrap_conan_target(Boost::unit_test_framework boost)
endfunction()
