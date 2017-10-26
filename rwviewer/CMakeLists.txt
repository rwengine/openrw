set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

add_executable(rwviewer
  main.cpp
  ViewerWindow.cpp

  models/ObjectListModel.cpp
  models/DFFFramesTreeModel.cpp

  views/ViewerInterface.hpp
  views/ObjectViewer.cpp
  views/ModelViewer.cpp
  views/WorldViewer.cpp

  ViewerWidget.cpp
  ItemListModel.cpp
  ItemListWidget.cpp
  IMGArchiveModel.cpp
  widgets/ModelFramesWidget.cpp
  AnimationListModel.cpp
  AnimationListWidget.cpp
  )

target_include_directories(rwviewer
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(rwviewer
  PRIVATE
  rwengine
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  )

install(TARGETS rwviewer RUNTIME DESTINATION "${BIN_DIR}")
