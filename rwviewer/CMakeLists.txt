set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)

add_executable(rwviewer
	main.cpp
	ViewerWindow.cpp
	ViewerWidget.cpp
	ItemListModel.cpp
	ItemListWidget.cpp
	IMGArchiveModel.cpp
	DFFFramesTreeModel.cpp
	ModelFramesWidget.cpp
	AnimationListModel.cpp
	AnimationListWidget.cpp)

include_directories("${CMAKE_SOURCE_DIR}/rwengine/include" ${BULLET_INCLUDE_DIR})

target_link_libraries(rwviewer rwengine sfml-graphics sfml-system GL GLEW ${BULLET_LIBRARIES} iconv)
qt5_use_modules(rwviewer Widgets OpenGL)

install(TARGETS rwviewer RUNTIME DESTINATION bin)
