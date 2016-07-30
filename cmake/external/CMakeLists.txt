##### External files

set(EXTERNAL_PREFIX "${CMAKE_BINARY_DIR}/external")

include(inih.cmake)
if(MINGW)
	include(mman-win32.cmake)
endif()
