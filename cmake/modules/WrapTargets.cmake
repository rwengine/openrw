function(rwdep_wrap_find_package TARGET INCLUDE_DIRS LINK_LIBS)
    add_library("TMP_${TARGET}" INTERFACE)
    target_include_directories("TMP_${TARGET}" SYSTEM
        INTERFACE
            ${INCLUDE_DIRS}
        )
    target_link_libraries("TMP_${TARGET}"
        INTERFACE
            ${LINK_LIBS}
        )
    add_library("rwdep::${TARGET}" ALIAS "TMP_${TARGET}")
endfunction()

function(rwdep_wrap_find_packages)
    if(BULLET_FOUND)
        rwdep_wrap_find_package(bullet "${BULLET_INCLUDE_DIR}" "${BULLET_LIBRARIES}")
    endif()

    if(FFMPEG_FOUND)
        rwdep_wrap_find_package(ffmpeg "${FFMPEG_INCLUDE_DIR}" "${FFMPEG_LIBRARIES}")
    endif()

    if(GLM_FOUND)
        rwdep_wrap_find_package(glm "${GLM_INCLUDE_DIR}" "")
    endif()

    if(OPENAL_FOUND)
        rwdep_wrap_find_package(OpenAL "${OPENAL_INCLUDE_DIR}" "${OPENAL_LIBRARY}")
    endif()

    if(SDL2_FOUND)
        rwdep_wrap_find_package(SDL2 "${SDL2_INCLUDE_DIR}" "${SDL2_LIBRARY}")
    endif()

    if(Boost_FOUND)
        rwdep_wrap_find_package(boost "${Boost_INCLUDE_DIRS}" "")
    endif()
    if(Boost_SYSTEM_FOUND)
        rwdep_wrap_find_package(boost_system "${Boost_INCLUDE_DIRS}" "${Boost_SYSTEM_LIBRARY}")
    endif()
    if(Boost_FILESYSTEM_FOUND)
        rwdep_wrap_find_package(boost_filesystem "${Boost_INCLUDE_DIRS}" "${Boost_FILESYSTEM_LIBRARY};rwdep::boost_system")
    endif()
    if(Boost_PROGRAM_OPTIONS_FOUND)
        rwdep_wrap_find_package(boost_program_options "${Boost_INCLUDE_DIRS}" "${Boost_PROGRAM_OPTIONS_LIBRARY};rwdep::boost_system")
    endif()
    if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
        rwdep_wrap_find_package(boost_unit_test_framework "${Boost_INCLUDE_DIRS}" "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
    endif()
    if(FREETYPE_FOUND)
        rwdep_wrap_find_package(freetype "${FREETYPE_INCLUDE_DIRS}" "${FREETYPE_LIBRARIES}")
    endif()
endfunction()

function(rwdep_wrap_conan_target TARGET CONAN_NAME)
    add_library("_rwdep_${TARGET}" INTERFACE)
    target_link_libraries("_rwdep_${TARGET}" INTERFACE "CONAN_PKG::${CONAN_NAME}")
    add_library("rwdep::${TARGET}" ALIAS "_rwdep_${TARGET}")
endfunction()

function(rwdep_wrap_conan_targets)
    rwdep_wrap_conan_target(boost boost)
    rwdep_wrap_conan_target(boost_program_options boost)
    rwdep_wrap_conan_target(boost_filesystem boost)
    rwdep_wrap_conan_target(boost_unit_test_framework boost)

    rwdep_wrap_conan_target(OpenAL openal)
    rwdep_wrap_conan_target(bullet bullet)
    rwdep_wrap_conan_target(glm glm)
    rwdep_wrap_conan_target(ffmpeg ffmpeg)
    rwdep_wrap_conan_target(SDL2 sdl2)
    if(BUILD_TOOLS)
        rwdep_wrap_conan_target(freetype freetype)
    endif()
endfunction()
