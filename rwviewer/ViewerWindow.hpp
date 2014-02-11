#pragma once 
#ifndef _VIEWERWINDOW_HPP_
#define _VIEWERWINDOW_HPP_
#include <QMainWindow>

class ArchiveContentsWidget;
class ViewerWidget;
class ViewerWindow : public QMainWindow
{
	Q_OBJECT
	
	ViewerWidget* viewer;
	ArchiveContentsWidget* archivewidget;
public:

	ViewerWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void openArchive(const QString& name);
	
    virtual void closeEvent(QCloseEvent*);

public slots:

	void openArchive();
	void updateTitle(const QString& name);

private slots:

	void openRecent();

private:
	
	QList<QAction*> recentArchives;
	QAction* recentSep;
	void updateRecentArchives();
};

#endif 