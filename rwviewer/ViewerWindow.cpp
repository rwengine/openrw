#include "ViewerWindow.hpp"
#include <engine/GameWorld.hpp>
#include "ViewerWidget.hpp"
#include "ArchiveContentsWidget.hpp"
#include "ModelFramesWidget.hpp"
#include "AnimationListWidget.hpp"
#include <QMenuBar>
#include <QFileDialog>
#include <QApplication>
#include <QSettings>
#include <fstream>

static int MaxRecentArchives = 5;

ViewerWindow::ViewerWindow(QWidget* parent, Qt::WindowFlags flags): QMainWindow(parent, flags)
{
	setMinimumSize(640, 480);
	
	viewer = new ViewerWidget();
	this->setCentralWidget(viewer);
	
	archivewidget = new ArchiveContentsWidget;
	archivewidget->setObjectName("archivewidget");
	this->addDockWidget(Qt::LeftDockWidgetArea, archivewidget);
	
	frameswidget = new ModelFramesWidget;
	frameswidget->setObjectName("frameswidget");
	this->addDockWidget(Qt::RightDockWidgetArea, frameswidget);

	animationswidget = new AnimationListWidget;
	animationswidget->setObjectName("animationswidget");
	this->addDockWidget(Qt::RightDockWidgetArea, animationswidget);
	
	QMenuBar* mb = this->menuBar();
	QMenu* file = mb->addMenu("&File");
	file->addAction("Open &Archive", this, SLOT(openArchive()));
	file->addSeparator();
	for(int i = 0; i < MaxRecentArchives; ++i) {
		QAction* r = file->addAction("");
		recentArchives.append(r);
		connect(r, SIGNAL(triggered()), SLOT(openRecent()));
	}
	recentSep = file->addSeparator();
	auto ex = file->addAction("E&xit");
	ex->setShortcut(QKeySequence::Quit);
	connect(ex, SIGNAL(triggered()), QApplication::instance(), SLOT(closeAllWindows()));

	QMenu* anim = mb->addMenu("&Animation");
	anim->addAction("Load &Animations", this, SLOT(openAnimations()));
	
	connect(archivewidget, SIGNAL(selectedFileChanged(QString)), viewer, SLOT(showFile(QString)));
	connect(animationswidget, SIGNAL(selectedAnimationChanged(Animation*)), viewer, SLOT(showAnimation(Animation*)));
	connect(viewer, SIGNAL(fileOpened(QString)), SLOT(openFileChanged(QString)));
	
	updateRecentArchives();
}

void ViewerWindow::openArchive(const QString& name)
{
	QString rname = name;
	QString lower = name.toLower();
	if(lower.endsWith(".img")) {
		rname = rname.left(rname.size()-4);
	}
	
	LoaderIMG ld;
	ld.load(rname.toStdString());
	
	viewer->world()->gameData.loadIMG(rname.toStdString());
	
	archivewidget->setArchive(ld);
	
	QSettings settings("OpenRW", "rwviewer");
	QStringList recent = settings.value("recentArchives").toStringList();
	recent.removeAll(name);
	recent.prepend(name);
	while(recent.size() > MaxRecentArchives) recent.removeLast();
	settings.setValue("recentArchives", recent);
	
	updateRecentArchives();
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

void ViewerWindow::openArchive()
{
	QFileDialog dialog(this, "Open Archive", QDir::homePath(), "IMG Archives (*.img)");
	if(dialog.exec()) {
		openArchive(dialog.selectedFiles().at(0));
	}
}

void ViewerWindow::openAnimations()
{
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
}

void ViewerWindow::openFileChanged(const QString& name)
{
	setWindowTitle(name);
	if(viewer->fileMode() == ViewerWidget::DFF) {
		frameswidget->setModel(viewer->currentModel());
	}
	else {
		frameswidget->setModel(nullptr);
	}
}

void ViewerWindow::openRecent()
{
	QAction* r = qobject_cast< QAction* >(sender());
	if(r) {
		openArchive(r->data().toString());
	}
}

void ViewerWindow::updateRecentArchives()
{
	QSettings settings("OpenRW", "rwviewer");
	QStringList recent = settings.value("recentArchives").toStringList();
	
	for(int i = 0; i < MaxRecentArchives; ++i) {
		if(i < recent.size()) {
			QString fnm(QFileInfo(recent[i]).fileName());
			recentArchives[i]->setText(tr("&%1 - %2").arg(i).arg(fnm));
			recentArchives[i]->setData(recent[i]);
			recentArchives[i]->setVisible(true);
		}
		else {
			recentArchives[i]->setVisible(false);
		}
	}
	
	recentSep->setVisible(recent.size() > 0);
}


