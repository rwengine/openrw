#pragma once
#ifndef _OBJECTVIEWER_HPP_
#define _OBJECTVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <QGLWidget>
#include <QTableView>
#include <QLabel>

class ViewerWidget;

class ObjectViewer : public QWidget
{
	Q_OBJECT

	QTableView* objectList;
	GameWorld* _world;

	ViewerWidget* previewWidget;
	QLabel* previewID;
	QLabel* previewModel;
	QLabel* previewClass;

public:

	ObjectViewer(ViewerWidget *viewer = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);

	GameWorld* world()
	{
		return _world;
	}

public slots:

	void showItem(qint16 item);

	void showData(GameWorld* world);

private slots:

	void showItem(QModelIndex model);

};

#endif
