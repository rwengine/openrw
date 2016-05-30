###### External Project: inih
include(ExternalProject)

SET(INIH_PREFIX_DIR ${PROJECT_SOURCE_DIR}/external/inih)

SET(INIH_SOURCES
	${INIH_PREFIX_DIR}/ini.c)
add_library(inih
	${INIH_SOURCES})

target_include_directories(inih INTERFACE SYSTEM ${INIH_PREFIX_DIR})
