#
# This script defines the following variables:
# - MAD_LIBRARY:    The mad library
# - MAD_FOUND:        true if all the required modules are found
# - MAD_INCLUDE_DIR:  the path where MAD headers are located
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

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MAD
	REQUIRED_VARS MAD_LIBRARY MAD_INCLUDE_DIR)

