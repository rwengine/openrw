add_executable(rwgame
	main.cpp

	RWGame.cpp

	State.cpp

	loadingstate.cpp
	ingamestate.cpp
	pausestate.cpp
	menustate.cpp
	debugstate.cpp
	benchmarkstate.cpp
	
	DrawUI.cpp

	debug/HttpServer.cpp
)

include_directories("${CMAKE_SOURCE_DIR}/rwengine/include" ${BULLET_INCLUDE_DIR})

target_link_libraries( rwgame
	rwengine
	sfml-graphics
	sfml-window
	sfml-network
	sfml-system
	${OPENGL_LIBRARIES}
	${BULLET_LIBRARIES})

install(TARGETS rwgame RUNTIME DESTINATION bin)
