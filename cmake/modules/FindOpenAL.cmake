# Override CMake's FindBullet module:
# create an IMPORTED TARGET

include("${CMAKE_ROOT}/Modules/FindOpenAL.cmake")

if(OPENAL_FOUND)
  if(APPLE)
    set(OPENAL_LIBRARY "${OPENAL_LIBRARY}/OpenAL")
  endif()

  add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
  set_target_properties(OpenAL::OpenAL PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "C;CXX"
    IMPORTED_LOCATION "${OPENAL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
    )
endif()
