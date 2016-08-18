#pragma once
#ifndef _MODELVIEWER_HPP_
#define _MODELVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include "AnimationListWidget.hpp"

#include "ViewerInterface.hpp"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLayout>

class ViewerWidget;
class Model;
class Skeleton;
class ModelFramesWidget;
class Animation;

class ModelViewer : public ViewerInterface
{
  Q_OBJECT

  Model* viewing;
  Skeleton* skeleton;

  QSplitter* mainSplit;
  QVBoxLayout* mainLayout;
  ViewerWidget* viewerWidget;

  ModelFramesWidget* frames;

  AnimationList loadedAnimations;
  AnimationListWidget* animationWidget;

public:
  ModelViewer(ViewerWidget* viewer = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);

  void setViewerWidget(ViewerWidget* widget) override;

public slots:

  /**
   * Display a raw model
   */
  void showModel(Model* model);

  /**
   * Display a game object's model
   */
  void showObject(uint16_t object);

  void loadAnimations(const QString& file);
  void playAnimation(Animation* anim);
};

#endif
