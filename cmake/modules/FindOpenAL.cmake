# Override CMake's FindOpenAL module:
# - create a OpenAL::OpenAL target

include("${CMAKE_ROOT}/Modules/FindOpenAL.cmake")

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
