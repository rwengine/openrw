add_executable(game main.cpp)

include_directories(../framework2/include /usr/include/bullet)

target_link_libraries(game renderware sfml-graphics sfml-window sfml-system GL GLEW BulletDynamics BulletCollision LinearMath )

install(TARGETS game RUNTIME DESTINATION bin)
