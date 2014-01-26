add_executable(rwviewer main.cpp)

include_directories(../rwengine/include /usr/include/bullet)

target_link_libraries(rwviewer rwengine sfml-graphics sfml-window sfml-system GL GLEW BulletDynamics BulletCollision LinearMath )

install(TARGETS rwviewer RUNTIME DESTINATION bin)
