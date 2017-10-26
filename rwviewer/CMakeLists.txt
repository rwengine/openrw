set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)

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

target_link_libraries(rwviewer
    rwengine
    )

qt5_use_modules(rwviewer Widgets OpenGL)

install(TARGETS rwviewer RUNTIME DESTINATION "${BIN_DIR}")
