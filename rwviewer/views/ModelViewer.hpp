#pragma once
#ifndef _MODELVIEWER_HPP_
#define _MODELVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <QGLWidget>
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLayout>

class ViewerWidget;
class Model;
class ModelFramesWidget;

class ModelViewer : public QWidget
{
	Q_OBJECT

	GameWorld* _world;
	Model* viewing;

	QSplitter* mainSplit;
	QVBoxLayout* mainLayout;
	ViewerWidget* viewerWidget;

	ModelFramesWidget* frames;
public:

	ModelViewer(ViewerWidget *viewer = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);

	GameWorld* world()
	{
		return _world;
	}

	void setViewerWidget( ViewerWidget* widget );

public slots:

	void showModel(Model* model);

	void showData(GameWorld* world);
};

#endif
