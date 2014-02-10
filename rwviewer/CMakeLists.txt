set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)

add_executable(rwviewer
	main.cpp
	ViewerWindow.cpp
	ViewerWidget.cpp)

include_directories(../rwengine/include /usr/include/bullet)

target_link_libraries(rwviewer rwengine sfml-graphics sfml-system GL GLEW BulletDynamics BulletCollision LinearMath)
qt5_use_modules(rwviewer Widgets OpenGL)

install(TARGETS rwviewer RUNTIME DESTINATION bin)
