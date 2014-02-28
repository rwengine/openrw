add_executable(rwgame main.cpp)

include_directories("${CMAKE_SOURCE_DIR}/rwengine/include" /usr/include/bullet)

target_link_libraries( rwgame rwengine sfml-graphics sfml-window sfml-system GL GLEW BulletDynamics BulletCollision LinearMath )

install(TARGETS rwgame RUNTIME DESTINATION bin)
