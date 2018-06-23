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
    add_library("CONAN_${CONAN_NAME}" INTERFACE)
    target_link_libraries("CONAN_${CONAN_NAME}" INTERFACE "CONAN_PKG::${CONAN_NAME}")
    add_library("${TARGET}" ALIAS "CONAN_${CONAN_NAME}")
endfunction()

function(rwdep_wrap_conan_targets)
    rwdep_wrap_conan_target(boost boost)
    rwdep_wrap_conan_target(boost_program_options boost)
    rwdep_wrap_conan_target(boost_filesystem boost)
    rwdep_wrap_conan_target(boost_unit_test_framework boost)

    rwdep_wrap_conan_target(OpenAL::OpenAL openal)
    rwdep_wrap_conan_target(bullet::bullet bullet)
    rwdep_wrap_conan_target(glm::glm glm)
    rwdep_wrap_conan_target(ffmpeg::ffmpeg ffmpeg)
    rwdep_wrap_conan_target(SDL2::SDL2 sdl2)
    if(BUILD_TOOLS)
        rwdep_wrap_conan_target(Freetype::Freetype freetype)
    endif()
endfunction()
