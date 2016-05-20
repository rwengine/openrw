###### External Project: inih
include(ExternalProject)

SET(INIH_REPOSITORY https://github.com/benhoyt/inih.git)
SET(INIH_PREFIX_DIR ${EXTERNAL_PREFIX}/inih)

ExternalProject_Add(inih-repository
	PREFIX ${INIH_PREFIX_DIR}
	# Do nothing, we just need the files.
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	GIT_REPOSITORY ${INIH_REPOSITORY})

SET(INIH_SOURCES
	${INIH_PREFIX_DIR}/src/inih-repository/ini.c)
set_source_files_properties(${INIH_SOURCES} PROPERTIES GENERATED TRUE)
add_library(inih
	${INIH_SOURCES})

add_dependencies(inih inih-repository)
target_include_directories(inih INTERFACE SYSTEM ${INIH_PREFIX_DIR}/src/inih-repository/)
