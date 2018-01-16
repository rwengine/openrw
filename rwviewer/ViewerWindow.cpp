#include "ViewerWindow.hpp"
#include <ViewerWidget.hpp>
#include "views/ModelViewer.hpp"
#include "views/ObjectViewer.hpp"
#include "views/WorldViewer.hpp"

#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>

#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QWindow>
#include <QMessageBox>

static int MaxRecentGames = 5;

ViewerWindow::ViewerWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , gameData(nullptr)
    , gameWorld(nullptr)
    , renderer(nullptr) {
    show();
    setMinimumSize(640, 480);
    createMenus();
    if (!setupEngine()) {
        return;
    }
    createDefaultViews();
}

void ViewerWindow::createMenus() {
    QMenuBar* mb = this->menuBar();
    QMenu* file = mb->addMenu("&File");

    file->addAction("Open &Game", this, SLOT(loadGame()));

    file->addSeparator();
    for (int i = 0; i < MaxRecentGames; ++i) {
        QAction* r = file->addAction("");
        recentGames.append(r);
        connect(r, SIGNAL(triggered()), SLOT(openRecent()));
    }

    recentSep = file->addSeparator();
    auto ex = file->addAction("E&xit");
    ex->setShortcut(QKeySequence::Quit);
    connect(ex, SIGNAL(triggered()), QApplication::instance(),
            SLOT(closeAllWindows()));

    mb->addMenu("&Data");

    updateRecentGames();
}

bool ViewerWindow::setupEngine() {
    QSurfaceFormat format = windowHandle()->format();
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3,3);
    context_ = new QOpenGLContext(this);
    context_->setShareContext(QOpenGLContext::globalShareContext());
    context_->setFormat(format);

    hiddenSurface = new QOffscreenSurface(windowHandle()->screen());
    hiddenSurface->setFormat(format);
    hiddenSurface->create();

    if (!context_->create()) {
        QMessageBox::critical(this, "OpenGL Failure",
                              "Failed to create OpenGL context");
        QApplication::exit(1);
        return false;
    }

    return true;
}

void ViewerWindow::createDefaultViews() {
    views = new QTabWidget(this);

    auto objectView = new ObjectViewer(this);
    views->addTab(objectView, "Objects");
    connect(this, &ViewerWindow::gameLoaded, objectView, &ObjectViewer::showData);

    auto modelView = new ModelViewer(this);
    views->addTab(modelView, "Model");
    connect(this, &ViewerWindow::gameLoaded, modelView, &ModelViewer::showData);
    connect(objectView, &ObjectViewer::showObjectModel, modelView, &ModelViewer::showObject);

    auto worldView = new WorldViewer(this);
    views->addTab(worldView, "World");
    connect(this, &ViewerWindow::gameLoaded, worldView, &WorldViewer::showData);

    setCentralWidget(views);
}

void ViewerWindow::showEvent(QShowEvent*) {
    static bool first = true;
    if (first) {
        QSettings settings("OpenRW", "rwviewer");
        restoreGeometry(settings.value("window/geometry").toByteArray());
        restoreState(settings.value("window/windowState").toByteArray());
        first = false;
    }
}

void ViewerWindow::closeEvent(QCloseEvent* event) {
    QSettings settings("OpenRW", "rwviewer");
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/windowState", saveState());
    QMainWindow::closeEvent(event);
}

void ViewerWindow::loadGame() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Open Directory"), QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.size() > 0) loadGame(dir);
}

void ViewerWindow::loadGame(const QString& path) {
    QDir gameDir(path);
    if (path.isEmpty()) {
        return;
    }
    if (!gameDir.exists()) {
        QMessageBox::critical(this, "Error", "The requested path doesn't exist");
        return;
    }

    if (!makeCurrent()) {
        return;
    }

    gameData = std::make_unique<GameData>(&engineLog, gameDir.absolutePath().toStdString());
    gameWorld = std::make_unique<GameWorld>(&engineLog, gameData.get());
    renderer = std::make_unique<GameRenderer>(&engineLog, gameData.get());
    gameWorld->state = new GameState;

    gameWorld->data->load();

    gameLoaded(gameWorld.get(), renderer.get());

    QSettings settings("OpenRW", "rwviewer");
    QStringList recent = settings.value("recentGames").toStringList();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > MaxRecentGames) recent.removeLast();
    settings.setValue("recentGames", recent);

    updateRecentGames();
}

void ViewerWindow::openRecent() {
    QAction* r = qobject_cast<QAction*>(sender());
    if (r) {
        loadGame(r->data().toString());
    }
}

void ViewerWindow::showObjectModel(uint16_t) {
#warning implement me
}

void ViewerWindow::updateRecentGames() {
    QSettings settings("OpenRW", "rwviewer");
    QStringList recent = settings.value("recentGames").toStringList();

    for (int i = 0; i < MaxRecentGames; ++i) {
        if (i < recent.size()) {
            QString fnm(QFileInfo(recent[i]).fileName());
            recentGames[i]->setText(tr("&%1 - %2").arg(i).arg(fnm));
            recentGames[i]->setData(recent[i]);
            recentGames[i]->setVisible(true);
        } else {
            recentGames[i]->setVisible(false);
        }
    }

    recentSep->setVisible(recent.size() > 0);
}

ViewerWindow::~ViewerWindow() {

}

bool ViewerWindow::makeCurrent() {
    if (!context_->makeCurrent(hiddenSurface)) {
        QMessageBox::critical(this, "OpenGL", "makeCurrent failed");
        QApplication::exit(1);
        return false;
    }
    return  true;
}

ViewerWidget *ViewerWindow::createViewer() {
    auto view = new ViewerWidget(context_, windowHandle());
    connect(this, &ViewerWindow::gameLoaded, view, &ViewerWidget::gameLoaded);
    return view;
}
