set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_executable(rwviewer
    main.cpp

    ViewerWindow.hpp
    ViewerWindow.cpp
    
    models/AnimationListModel.hpp
    models/AnimationListModel.cpp
    models/ObjectListModel.hpp
    models/ObjectListModel.cpp
    models/DFFFramesTreeModel.hpp
    models/DFFFramesTreeModel.cpp
    models/IMGArchiveModel.hpp
    models/IMGArchiveModel.cpp
    models/ItemListModel.hpp
    models/ItemListModel.cpp
    models/TextModel.hpp
    models/TextModel.cpp

    views/ObjectViewer.hpp
    views/ObjectViewer.cpp
    views/ModelViewer.hpp
    views/ModelViewer.cpp
    views/TextViewer.hpp
    views/TextViewer.cpp
    views/WorldViewer.hpp
    views/WorldViewer.cpp
    views/ViewerInterface.hpp
    views/ViewerInterface.cpp

    QOpenGLContextWrapper.hpp
    QOpenGLContextWrapper.cpp
    ViewerWidget.hpp
    ViewerWidget.cpp

    widgets/AnimationListWidget.hpp
    widgets/AnimationListWidget.cpp
    widgets/ItemListWidget.hpp
    widgets/ItemListWidget.cpp
    widgets/ModelFramesWidget.hpp
    widgets/ModelFramesWidget.cpp
    )

target_link_libraries(rwviewer
    PRIVATE
        rwengine
        Qt5::OpenGL
        Qt5::Widgets
    )

openrw_target_apply_options(
    TARGET rwviewer
    FORMAT
    INSTALL INSTALL_PDB
    )

if(USE_CONAN)
    set(QT5_ROOT "${CONAN_QT_ROOT}")
    configure_file(qt.conf.in qt.conf)
endif()
