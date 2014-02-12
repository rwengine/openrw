#include "ViewerWindow.hpp"
#include <engine/GameWorld.hpp>
#include "ViewerWidget.hpp"
#include "ArchiveContentsWidget.hpp"
#include "ModelFramesWidget.hpp"
#include <QMenuBar>
#include <QFileDialog>
#include <QApplication>
#include <QSettings>

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
	file->addAction("E&xit", QApplication::instance(), SLOT(quit()), QKeySequence::Quit);
	
	connect(archivewidget, SIGNAL(selectedFileChanged(QString)), viewer, SLOT(showFile(QString)));
	connect(viewer, SIGNAL(fileOpened(QString)), SLOT(openFileChanged(QString)));
	
	QSettings settings("OpenRW", "rwviewer");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
	
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

void ViewerWindow::closeEvent(QCloseEvent* event)
{
	QSettings settings("OpenRW", "rwviewer");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	QMainWindow::closeEvent(event);
}

void ViewerWindow::openArchive()
{
	QFileDialog dialog(this, "Open Archive", QDir::homePath(), "IMG Archives (*.img)");
	if(dialog.exec()) {
		openArchive(dialog.selectedFiles().at(0));
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


