#include "ViewerWindow.hpp"
#include "views/ObjectViewer.hpp"
#include "views/ModelViewer.hpp"

#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>
#include <QMenuBar>
#include <QFileDialog>
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <fstream>
#include <QOffscreenSurface>
#include <ViewerWidget.hpp>

static int MaxRecentGames = 5;

ViewerWindow::ViewerWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), gameData(nullptr), gameWorld(nullptr), renderer(nullptr)
{
	setMinimumSize(640, 480);

	QMenuBar* mb = this->menuBar();
	QMenu* file = mb->addMenu("&File");

	file->addAction("Open &Game", this, SLOT(loadGame()));

	file->addSeparator();
	for(int i = 0; i < MaxRecentGames; ++i) {
		QAction* r = file->addAction("");
		recentGames.append(r);
		connect(r, SIGNAL(triggered()), SLOT(openRecent()));
	}

	recentSep = file->addSeparator();
	auto ex = file->addAction("E&xit");
	ex->setShortcut(QKeySequence::Quit);
	connect(ex, SIGNAL(triggered()), QApplication::instance(), SLOT(closeAllWindows()));

	viewerWidget = new ViewerWidget;

	viewerWidget->context()->makeCurrent();

	glewExperimental = 1;
	glewInit();

	objectViewer = new ObjectViewer(viewerWidget);

	connect(this, SIGNAL(loadedData(GameWorld*)), objectViewer, SLOT(showData(GameWorld*)));
	connect(this, SIGNAL(loadedData(GameWorld*)), viewerWidget, SLOT(dataLoaded(GameWorld*)));

	modelViewer = new ModelViewer(viewerWidget);

	connect(this, SIGNAL(loadedData(GameWorld*)), modelViewer, SLOT(showData(GameWorld*)));

	viewSwitcher = new QStackedWidget;
	viewSwitcher->addWidget(objectViewer);
	viewSwitcher->addWidget(modelViewer);

	//connect(objectViewer, SIGNAL(modelChanged(Model*)), modelViewer, SLOT(showModel(Model*)));
	connect(objectViewer, SIGNAL(showObjectModel(uint16_t)), this, SLOT(showObjectModel(uint16_t)));

	objectViewer->setViewerWidget( viewerWidget );

	QMenu* view = mb->addMenu("&View");
	QAction* objectAction = view->addAction("&Object");
	QAction* modelAction = view->addAction("&Model");

	objectAction->setData(0);
	modelAction->setData(1);

	connect(objectAction, SIGNAL(triggered()), this, SLOT(switchWidget()));
	connect(modelAction, SIGNAL(triggered()), this, SLOT(switchWidget()));

	QMenu* data = mb->addMenu("&Data");
	//data->addAction("Export &Model", objectViewer, SLOT(exportModel()));

	QMenu* anim = mb->addMenu("&Animation");
	anim->addAction("Load &Animations", this, SLOT(openAnimations()));

	this->setCentralWidget(viewSwitcher);

	updateRecentGames();
}

void ViewerWindow::showEvent(QShowEvent*)
{
	static bool first = true;
	if(first) {
		QSettings settings("OpenRW", "rwviewer");
		restoreGeometry(settings.value("window/geometry").toByteArray());
		restoreState(settings.value("window/windowState").toByteArray());
		first = false;
	}
}

void ViewerWindow::closeEvent(QCloseEvent* event)
{
	QSettings settings("OpenRW", "rwviewer");
	settings.setValue("window/geometry", saveGeometry());
	settings.setValue("window/windowState", saveState());
	QMainWindow::closeEvent(event);
}

void ViewerWindow::openAnimations()
{
#if 0
	QFileDialog dialog(this, "Open Animations", QDir::homePath(), "IFP Animations (*.ifp)");
	if(dialog.exec()) {
		std::ifstream dfile(dialog.selectedFiles().at(0).toStdString().c_str());
		AnimationList anims;

		if(dfile.is_open())
		{
			dfile.seekg(0, std::ios_base::end);
			size_t length = dfile.tellg();
			dfile.seekg(0);
			char *file = new char[length];
			dfile.read(file, length);

			LoaderIFP loader;
			if( loader.loadFromMemory(file) ) {
				for(auto& f : loader.animations) {
					anims.push_back(f);
				}
			}

			delete[] file;
		}

		animationswidget->setAnimations(anims);
	}
#endif
}

void ViewerWindow::loadGame()
{
	QString dir = QFileDialog::getExistingDirectory(
		this, tr("Open Directory"),
		 QDir::homePath(),
		 QFileDialog::ShowDirsOnly
		 | QFileDialog::DontResolveSymlinks);

	if( dir.size() > 0 ) loadGame( dir );
}

void ViewerWindow::loadGame(const QString &path)
{
	QDir gameDir( path );

	if( gameDir.exists() && path.size() > 0 ) {
		gameData = new GameData( &engineLog, &work, gameDir.absolutePath().toStdString() );
		gameWorld = new GameWorld( &engineLog, &work, gameData );
		renderer = new GameRenderer(&engineLog, gameData );
		viewerWidget->setRenderer(renderer);

		gameWorld->data->load();

		// Initalize all the archives.
		gameWorld->data->loadIMG("/models/gta3");
		gameWorld->data->loadIMG("/models/txd");
		gameWorld->data->loadIMG("/anim/cuts");

		loadedData(gameWorld);
	}

	QSettings settings("OpenRW", "rwviewer");
	QStringList recent = settings.value("recentGames").toStringList();
	recent.removeAll( path );
	recent.prepend( path );
	while(recent.size() > MaxRecentGames) recent.removeLast();
	settings.setValue("recentGames", recent);

	updateRecentGames();
}

void ViewerWindow::openRecent()
{
	QAction* r = qobject_cast< QAction* >(sender());
	if(r) {
		loadGame( r->data().toString() );
	}
}

void ViewerWindow::switchWidget()
{
	QAction* r = qobject_cast< QAction* >(sender());
	if(r) {
		int index = r->data().toInt();

		if( index == 0 )
		{
			objectViewer->setViewerWidget( viewerWidget );
		}
		else if( index == 1 )
		{
			modelViewer->setViewerWidget( viewerWidget );
		}

		viewSwitcher->setCurrentIndex( index );
	}
}

void ViewerWindow::showObjectModel(uint16_t object)
{
	// Switch to the model viewer
	modelViewer->setViewerWidget( viewerWidget );
	viewSwitcher->setCurrentIndex( viewSwitcher->indexOf(modelViewer) );
	modelViewer->showObject(object);
}

void ViewerWindow::updateRecentGames()
{
	QSettings settings("OpenRW", "rwviewer");
	QStringList recent = settings.value("recentGames").toStringList();
	
	for(int i = 0; i < MaxRecentGames; ++i) {
		if(i < recent.size()) {
			QString fnm(QFileInfo(recent[i]).fileName());
			recentGames[i]->setText(tr("&%1 - %2").arg(i).arg(fnm));
			recentGames[i]->setData(recent[i]);
			recentGames[i]->setVisible(true);
		}
		else {
			recentGames[i]->setVisible(false);
		}
	}
	
	recentSep->setVisible(recent.size() > 0);
}


