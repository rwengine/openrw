#pragma once
#ifndef _VIEWERWINDOW_HPP_
#define _VIEWERWINDOW_HPP_
#include <core/Logger.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <array>

class ViewerWidget;
class ViewerInterface;
class GameRenderer;
class QGLContext;

class ViewerWindow : public QMainWindow {
    Q_OBJECT

    enum ViewMode { Object = 0, Model = 1, World = 2, _Count };

    Logger engineLog;
    WorkContext work;

    GameData* gameData;
    GameWorld* gameWorld;
    GameRenderer* renderer;
    GameState* state;

    /** Contains the OGL context */
    ViewerWidget* viewerWidget;

    std::array<ViewerInterface*, ViewMode::_Count> m_views;
    std::array<std::string, ViewMode::_Count> m_viewNames;

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

    void loadGame(const QString& path);

signals:

    void loadedData(GameWorld* world);
    void loadAnimations(const QString& file);

private slots:

    void openRecent();

    void switchView(int mode);

    void showObjectModel(uint16_t object);

private:
    QList<QAction*> recentGames;
    QAction* recentSep;
    void updateRecentGames();
};

#endif
