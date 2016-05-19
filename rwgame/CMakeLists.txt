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

include_directories(SYSTEM
	${BULLET_INCLUDE_DIR}
	${SFML_INCLUDE_DIR}
	)
include_directories(
	"${CMAKE_SOURCE_DIR}/rwengine/include"
	)

target_link_libraries( rwgame
	rwengine
	${SFML_LIBRARIES}
	${OPENGL_LIBRARIES}
	${BULLET_LIBRARIES})

install(TARGETS rwgame RUNTIME DESTINATION bin)
