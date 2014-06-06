#pragma once
#ifndef _MODELFRAMESWIDGET_HPP_
#define _MODELFRAMESWIDGET_HPP_
#include <QDockWidget>
#include <QTreeView>
#include "DFFFramesTreeModel.hpp"
#include <engine/GTATypes.hpp>

class ModelFramesWidget : public QDockWidget
{
	Q_OBJECT

	ModelHandle* gmodel;
	DFFFramesTreeModel* framemodel;
	QTreeView* tree;
	
public:
    ModelFramesWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setModel(ModelHandle* model);
};

#endif
