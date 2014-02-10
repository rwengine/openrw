#pragma once
#ifndef _ARCHIVECONTENTSWIDGET_HPP_
#define _ARCHIVECONTENTSWIDGET_HPP_
#include <QDockWidget>
#include <QListView>
#include "IMGArchiveModel.hpp"

class ArchiveContentsWidget : public QDockWidget
{
	Q_OBJECT

	IMGArchiveModel* model;
	QListView* table;
	
public:
    ArchiveContentsWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setArchive(const LoaderIMG& archive);
	
signals:
	void selectedFileChanged(const std::string& file);

public slots:
	void selectedIndexChanged(const QModelIndex& current);
};

#endif