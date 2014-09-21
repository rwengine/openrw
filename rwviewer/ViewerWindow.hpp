#pragma once 
#ifndef _VIEWERWINDOW_HPP_
#define _VIEWERWINDOW_HPP_
#include <QMainWindow>
#include <engine/GameWorld.hpp>
#include <QGLContext>
#include <QStackedWidget>
#include <QVBoxLayout>

class ObjectViewer;
class ViewerWidget;

class ViewerWindow : public QMainWindow
{
	Q_OBJECT
	
	GameWorld* gameWorld;

	/** Contains the OGL context */
	ViewerWidget* viewerWidget;

	ObjectViewer* objectViewer;

	QVBoxLayout* modelLayout;

	QStackedWidget* viewSwitcher;

	QGLContext* context;
public:

	ViewerWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	/**
	 * @brief openGame Loads a game's dat file.
	 * @param datFile
	 */
	void openGame(const QString& datFile);

	virtual void showEvent(QShowEvent*);
	
	virtual void closeEvent(QCloseEvent*);

public slots:

	void openAnimations();

	void loadGame();

	void loadGame( const QString& path );

signals:

	void loadedData(GameWorld* world);

private slots:

	void openRecent();

	void switchWidget();

private:
	
	QList<QAction*> recentGames;
	QAction* recentSep;
	void updateRecentGames();
};

#endif 
