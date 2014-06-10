#pragma once
#ifndef _MODELFRAMESWIDGET_HPP_
#define _MODELFRAMESWIDGET_HPP_
#include <QDockWidget>
#include <QTreeView>
#include "DFFFramesTreeModel.hpp"
#include <engine/RWTypes.hpp>
#include <QVBoxLayout>
#include <QLabel>

class ModelFrame;

class ModelFramesWidget : public QDockWidget
{
	Q_OBJECT

	Model* gmodel;
	DFFFramesTreeModel* framemodel;
	QTreeView* tree;
	QVBoxLayout* _layout;
	QLabel* _frameLabel;

private slots:

	void updateInfoBox(ModelFrame* f);

	void selectedModelChanged(const QModelIndex&,const QModelIndex&);
	
public:
    ModelFramesWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:

	void setModel(Model *model);
};

#endif
