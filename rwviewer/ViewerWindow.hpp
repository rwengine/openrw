#pragma once 
#ifndef _VIEWERWINDOW_HPP_
#define _VIEWERWINDOW_HPP_
#include <QMainWindow>

class ModelFramesWidget;
class ArchiveContentsWidget;
class AnimationListWidget;
class ViewerWidget;

class ViewerWindow : public QMainWindow
{
	Q_OBJECT
	
	ViewerWidget* viewer;
	ArchiveContentsWidget* archivewidget;
	ModelFramesWidget* frameswidget;
	AnimationListWidget* animationswidget;
public:

	ViewerWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void openArchive(const QString& name);
	
    virtual void showEvent(QShowEvent*);
	
    virtual void closeEvent(QCloseEvent*);

public slots:

	void openArchive();

	void openAnimations();

private slots:

	void openFileChanged(const QString& name);
	void openRecent();

private:
	
	QList<QAction*> recentArchives;
	QAction* recentSep;
	void updateRecentArchives();
};

#endif 
