#ifndef _MODELVIEWER_HPP_
#define _MODELVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>

#include "ViewerInterface.hpp"

#include <QLabel>
#include <QLayout>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

class ViewerWidget;
class Clump;
class ModelFramesWidget;

class ModelViewer : public ViewerInterface {
    Q_OBJECT

    ClumpPtr viewing;

    QSplitter* mainSplit;
    QVBoxLayout* mainLayout;
    ViewerWidget* viewerWidget;

    ModelFramesWidget* frames;

public:
    ModelViewer(QWidget* parent = 0, Qt::WindowFlags f = 0);

public slots:

    /**
     * Display a raw model
     */
    void showModel(ClumpPtr model);

    /**
     * Display a game object's model
     */
    void showObject(uint16_t object);
};

#endif
