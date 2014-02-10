#pragma once
#ifndef _ARCHIVECONTENTSWIDGET_HPP_
#define _ARCHIVECONTENTSWIDGET_HPP_
#include <QDockWidget>
#include <QTableView>
#include "IMGTableModel.hpp"

class ArchiveContentsWidget : public QDockWidget
{
	Q_OBJECT

	IMGTableModel* model;
	QTableView* table;
	
public:
    ArchiveContentsWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setArchive(const LoaderIMG& archive);
};

#endif