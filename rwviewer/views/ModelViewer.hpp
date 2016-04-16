#pragma once
#ifndef _MODELVIEWER_HPP_
#define _MODELVIEWER_HPP_
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLayout>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include "AnimationListWidget.hpp"

class ViewerWidget;
class Model;
class Skeleton;
class ModelFramesWidget;
class Animation;

class ModelViewer : public QWidget
{
	Q_OBJECT

	GameWorld* _world;
	Model* viewing;
	Skeleton* skeleton;

	QSplitter* mainSplit;
	QVBoxLayout* mainLayout;
	ViewerWidget* viewerWidget;

	ModelFramesWidget* frames;

	AnimationList loadedAnimations;
	AnimationListWidget *animationWidget;
public:

	ModelViewer(ViewerWidget *viewer = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);

	GameWorld* world()
	{
		return _world;
	}

	void setViewerWidget( ViewerWidget* widget );

public slots:

	/**
	 * Display a raw model
	 */
	void showModel(Model* model);

	/**
	 * Display a game object's model
	 */
	void showObject(uint16_t object);

	void showData(GameWorld* world);

	void loadAnimations(const QString& file);
	void playAnimation(Animation* anim);
};

#endif
