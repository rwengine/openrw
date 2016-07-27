include(ExternalProject)
SET(MMAN_W32_DIR ${PROJECT_SOURCE_DIR}/external/mman-win32)

SET(MMAN_W32_SOURCES
	${MMAN_W32_DIR}/mman.c)
file(GLOB MMAN_W32_HEADERS
	${MMAN_W32_DIR}/mman.h)

add_library(mman
	${MMAN_W32_SOURCES})
find_path(MINGWW64_ROOT NAMES include/sys/time.h PATH_SUFFIXES i686-w64-mingw32 mingw mingw32 )
if (MINGWW64_ROOT)
  message(STATUS "MINGWW64_ROOT found - ${MINGWW64_ROOT}")
else()
  message(FATAL_ERROR "MINGWW64_ROOT not found")
endif()

target_include_directories(mman INTERFACE SYSTEM ${MMAN_W32_DIR})
set_target_properties( mman
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${MINGWW64_ROOT}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${MINGWW64_ROOT}/lib"    
)
add_custom_command(
      TARGET mman
      COMMAND ${CMAKE_COMMAND} -E copy "${MMAN_W32_HEADERS}" "${MINGWW64_ROOT}/include/sys"
	  COMMENT "Installing headers for libmman-win32..."
)