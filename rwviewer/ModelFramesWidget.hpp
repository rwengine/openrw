#pragma once
#ifndef _MODELFRAMESWIDGET_HPP_
#define _MODELFRAMESWIDGET_HPP_
#include <QDockWidget>
#include <QTreeView>
#include "DFFFramesTreeModel.hpp"
#include <engine/RWTypes.hpp>

class ModelFramesWidget : public QDockWidget
{
	Q_OBJECT

	Model* gmodel;
	DFFFramesTreeModel* framemodel;
	QTreeView* tree;
	
public:
    ModelFramesWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:

	void setModel(Model *model);
};

#endif
