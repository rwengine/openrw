add_executable(rwgame
	main.cpp

	State.cpp

	loadingstate.cpp
	ingamestate.cpp
	pausestate.cpp
	menustate.cpp
	debugstate.cpp
)

include_directories("${CMAKE_SOURCE_DIR}/rwengine/include" ${BULLET_INCLUDE_DIR})

target_link_libraries( rwgame rwengine sfml-graphics sfml-window sfml-system ${OPENGL_LIBRARIES} GLEW ${BULLET_LIBRARIES} iconv )

install(TARGETS rwgame RUNTIME DESTINATION bin)
