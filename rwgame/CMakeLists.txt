configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)

set(RWGAME_SOURCES
	GitSHA1.h
	"${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp"

	main.cpp

	RWGame.cpp

	GameConfig.cpp
	GameWindow.cpp

	State.cpp

	loadingstate.cpp
	ingamestate.cpp
	pausestate.cpp
	menustate.cpp
	debugstate.cpp
	benchmarkstate.cpp
	
	DrawUI.cpp
)

add_executable(rwgame
	${RWGAME_SOURCES})

include_directories(SYSTEM
	${BULLET_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
	)
include_directories(
	"${CMAKE_SOURCE_DIR}/rwengine/include"
	)

target_link_libraries(rwgame
	rwengine
	inih
	${OPENGL_LIBRARIES}
	${BULLET_LIBRARIES}
	${SDL2_LIBRARY}
	)
if(MINGW)
	add_definitions(-D _USE_MATH_DEFINES)
	target_link_libraries(rwgame
	iconv
	mman)
endif()

install(TARGETS rwgame RUNTIME DESTINATION "${BIN_DIR}")
