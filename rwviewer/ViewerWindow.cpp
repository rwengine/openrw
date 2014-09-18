#include "ViewerWindow.hpp"
#include "ObjectViewer.hpp"

#include <engine/GameWorld.hpp>
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
	: QMainWindow(parent, flags), gameWorld(nullptr)
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

	QMenu* data = mb->addMenu("&Data");
	//data->addAction("Export &Model", objectViewer, SLOT(exportModel()));

	QMenu* anim = mb->addMenu("&Animation");
	anim->addAction("Load &Animations", this, SLOT(openAnimations()));

	viewerWidget = new ViewerWidget;

	viewerWidget->context()->makeCurrent();

	glewExperimental = 1;
	glewInit();

	objectViewer = new ObjectViewer(viewerWidget);
	this->setCentralWidget(objectViewer);

	connect(this, SIGNAL(loadedData(GameWorld*)), objectViewer, SLOT(showData(GameWorld*)));
	connect(this, SIGNAL(loadedData(GameWorld*)), viewerWidget, SLOT(dataLoaded(GameWorld*)));

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
		gameWorld = new GameWorld( gameDir.absolutePath().toStdString() );
		gameWorld->load();

		// Initalize all the archives.
		gameWorld->gameData.loadIMG("/models/gta3");
		gameWorld->gameData.loadIMG("/models/txd");
		gameWorld->gameData.loadIMG("/anim/cuts");

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


