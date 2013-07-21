add_executable(viewer main.cpp)

include_directories(../framework2/include /usr/include/bullet)

target_link_libraries( viewer renderware sfml-graphics sfml-window sfml-system GL GLEW BulletDynamics BulletCollision LinearMath )

install(TARGETS viewer RUNTIME DESTINATION bin)
