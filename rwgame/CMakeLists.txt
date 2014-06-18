add_executable(rwgame
	main.cpp
	loadingstate.cpp
	ingamestate.cpp
	pausestate.cpp
	menustate.cpp
	debugstate.cpp
)

include_directories("${CMAKE_SOURCE_DIR}/rwengine/include" ${BULLET_INCLUDE_DIRS})

target_link_libraries( rwgame rwengine sfml-graphics sfml-window sfml-system GL GLEW ${BULLET_LIBRARIES} )

install(TARGETS rwgame RUNTIME DESTINATION bin)
