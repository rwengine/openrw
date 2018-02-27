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
endfunction()
