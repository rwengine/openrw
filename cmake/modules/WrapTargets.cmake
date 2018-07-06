function(rwdep_wrap_find_packages)
    if(BULLET_FOUND AND NOT TARGET bullet::bullet)
        add_library(bullet INTERFACE)
        target_link_libraries(bullet
            INTERFACE
                ${BULLET_LIBRARIES}
            )
        target_include_directories(bullet SYSTEM
            INTERFACE
                "${BULLET_INCLUDE_DIR}"
            )
        add_library(bullet::bullet ALIAS bullet)
    endif()

    if(OPENAL_FOUND AND NOT TARGET OpenAL::OpenAL)
        add_library(OpenAL INTERFACE)
        target_link_libraries(OpenAL
            INTERFACE
                "${OPENAL_LIBRARY}"
            )
        target_include_directories(OpenAL SYSTEM
            INTERFACE
                "${OPENAL_INCLUDE_DIR}"
            )
        add_library(OpenAL::OpenAL ALIAS OpenAL)
    endif()
endfunction()

function(rwdep_wrap_conan_target TARGET CONAN_NAME)
    string(RANDOM RND)
    set(TGT "CONAN_${CONAN_NAME}_${RND}")
    add_library("${TGT}" INTERFACE)
    target_link_libraries("${TGT}" INTERFACE "CONAN_PKG::${CONAN_NAME}")
    add_library("${TARGET}" ALIAS "${TGT}")
endfunction()

function(rwdep_wrap_conan_targets)
    rwdep_wrap_conan_target(OpenAL::OpenAL openal)
    rwdep_wrap_conan_target(bullet::bullet bullet)
    rwdep_wrap_conan_target(glm::glm glm)
    rwdep_wrap_conan_target(ffmpeg::ffmpeg ffmpeg)
    rwdep_wrap_conan_target(SDL2::SDL2 sdl2)
    if(BUILD_TOOLS)
        rwdep_wrap_conan_target(Freetype::Freetype freetype)
    endif()

    rwdep_wrap_conan_target(Boost::boost boost)
    rwdep_wrap_conan_target(Boost::filesystem boost)
    rwdep_wrap_conan_target(Boost::program_options boost)
    rwdep_wrap_conan_target(Boost::system boost)
    rwdep_wrap_conan_target(Boost::unit_test_framework boost)
endfunction()
