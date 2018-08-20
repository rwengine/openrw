set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_executable(rwviewer
    main.cpp

    ViewerWindow.hpp
    ViewerWindow.cpp

    models/ObjectListModel.hpp
    models/ObjectListModel.cpp
    models/DFFFramesTreeModel.hpp
    models/DFFFramesTreeModel.cpp
    models/TextModel.hpp
    models/TextModel.cpp

    views/ViewerInterface.hpp
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

    ViewerWidget.cpp
    ViewerWidget.hpp
    ItemListModel.hpp
    ItemListModel.cpp
    ItemListWidget.hpp
    ItemListWidget.cpp
    IMGArchiveModel.hpp
    IMGArchiveModel.cpp
    widgets/ModelFramesWidget.hpp
    widgets/ModelFramesWidget.cpp
    AnimationListModel.hpp
    AnimationListModel.cpp
    AnimationListWidget.hpp
    AnimationListWidget.cpp
    QOpenGLContextWrapper.cpp
    QOpenGLContextWrapper.hpp
    )

target_link_libraries(rwviewer
    PRIVATE
        rwengine
        Qt5::OpenGL
        Qt5::Widgets
    )

openrw_target_apply_options(TARGET rwviewer)

install(TARGETS rwviewer
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

if(USE_CONAN)
    set(QT5_ROOT "${CONAN_QT_ROOT}")
    configure_file(qt.conf.in qt.conf)
endif()
