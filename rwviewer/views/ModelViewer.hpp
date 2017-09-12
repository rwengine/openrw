#pragma once
#ifndef _MODELVIEWER_HPP_
#define _MODELVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include "AnimationListWidget.hpp"

#include "ViewerInterface.hpp"

#include <QLabel>
#include <QLayout>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

class ViewerWidget;
class Clump;
class ModelFramesWidget;
class Animation;

class ModelViewer : public ViewerInterface {
    Q_OBJECT

    ClumpPtr viewing;

    QSplitter* mainSplit;
    QVBoxLayout* mainLayout;
    ViewerWidget* viewerWidget;

    ModelFramesWidget* frames;

    AnimationList loadedAnimations;
    AnimationListWidget* animationWidget;

public:
    ModelViewer(ViewerWidget* viewer = 0, QWidget* parent = 0,
                Qt::WindowFlags f = 0);

    void setViewerWidget(ViewerWidget* widget) override;

public slots:

    /**
     * Display a raw model
     */
    void showModel(ClumpPtr model);

    /**
     * Display a game object's model
     */
    void showObject(uint16_t object);

    void loadAnimations(const QString& file);
    void playAnimation(Animation* anim);
};

#endif
