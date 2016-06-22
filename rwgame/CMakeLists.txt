add_executable(rwgame
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

	debug/HttpServer.cpp
	debug/TcpSocket.cpp
)

include_directories(SYSTEM
	${BULLET_INCLUDE_DIR}
	)
include_directories(
	"${CMAKE_SOURCE_DIR}/rwengine/include"
	)

target_link_libraries(rwgame
	rwengine
	inih
	${OPENGL_LIBRARIES}
	${BULLET_LIBRARIES}
	${SDL2_LIBRARIES}
	${PNG_LIBRARIES}
	)

install(TARGETS rwgame RUNTIME DESTINATION bin)
