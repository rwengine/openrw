#pragma once
#ifndef _ARCHIVECONTENTSWIDGET_HPP_
#define _ARCHIVECONTENTSWIDGET_HPP_
#include <QDockWidget>
#include <QListView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include "IMGArchiveModel.hpp"

class ArchiveContentsWidget : public QDockWidget
{
	Q_OBJECT

	QSortFilterProxyModel* filter;
	IMGArchiveModel* model;
	QListView* table;
	QLineEdit* searchbox;
	
public:
    ArchiveContentsWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setArchive(const LoaderIMG& archive);
	
signals:
	void selectedFileChanged(const QString& file);

public slots:
	void selectedIndexChanged(const QModelIndex& current);

	void setFilter(const QString& f);
};

#endif
