#pragma once
#ifndef _ANIMATIONLISTWIDGET_HPP_
#define _ANIMATIONLISTWIDGET_HPP_
#include <QDockWidget>
#include <QListView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include "AnimationListModel.hpp"

class AnimationListWidget : public QDockWidget
{
	Q_OBJECT

	QSortFilterProxyModel* filter;
	AnimationListModel* model;
	QListView* table;
	QLineEdit* searchbox;
	
public:

	AnimationListWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setAnimations(const AnimationList& anims);
	
signals:

	void selectedAnimationChanged(Animation* anim);

public slots:
	void selectedIndexChanged(const QModelIndex& current);

	void setFilter(const QString& f);
};

#endif
