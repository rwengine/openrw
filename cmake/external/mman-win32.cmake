###### External Project: mman-win32
include(ExternalProject)

SET(MMAN_WIN32_PREFIX_DIR ${PROJECT_SOURCE_DIR}/external/mman-win32)

SET(MMAN_WIN32_SOURCES
	${MMAN_WIN32_PREFIX_DIR}/mman.c)
add_library(mman
	${MMAN_WIN32_SOURCES})

# The source folder for mman has mman.h, however, we require sys/mman.h
# To get this, we copy it to the build directory.. This gives us a virtual sys/ folder.
add_custom_command(
	TARGET mman
	COMMAND ${CMAKE_COMMAND} -E copy "${MMAN_WIN32_PREFIX_DIR}/mman.h" "${CMAKE_CURRENT_BINARY_DIR}/include/sys/mman.h"
)

target_include_directories(mman INTERFACE SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/include)
