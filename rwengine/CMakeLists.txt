FILE(GLOB_RECURSE RENDERWARE_SOURCES "src/*.cpp")
FILE(GLOB_RECURSE RENDERWARE_HEADERS "include/*.hpp")

source_group("Header Files" FILES ${RENDERWARE_HEADERS})
source_group("Source Files" FILES ${RENDERWARE_SOURCES})

add_library(rwengine
    ${RENDERWARE_SOURCES}
    ${RENDERWARE_HEADERS}
)

target_link_libraries(rwengine
	rwlib
	sfml-window
	sfml-audio
	mad
	${OPENRW_PLATFORM_LIBS})

include_directories(SYSTEM
	${BULLET_INCLUDE_DIR}
	)

include_directories(
	include
	)
