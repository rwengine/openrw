#
# This script defines the following variables:
# - MAD_LIBRARY:    The mad library
# - MAD_FOUND:        true if all the required modules are found
# - MAD_INCLUDE_DIR:  the path where SFML headers are located (the directory containing the SFML/Config.hpp file)
#

set(FIND_MAD_PATHS
	${MAD_ROOT}
	$ENV{MAD_ROOT}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw
	/opt/local
	/opt/csw
	/opt)

find_path(MAD_INCLUDE_DIR mad.h
		PATH_SUFFIXES include
		PATHS ${FIND_MAD_PATHS})

find_library(MAD_LIBRARY
				NAMES mad
				PATH_SUFFIXES lib64 lib
				PATHS ${FIND_MAD_PATHS})

if (MAD_INCLUDE_DIR AND MAD_LIBRARY)
	set (MAD_FOUND TRUE)
endif ()

if (MAD_FOUND AND NOT MAD_FIND_QUIETLY)
	message(STATUS "Found MAD in ${MAD_INCLUDE_DIR}")
endif()
