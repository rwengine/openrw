#include "ViewerWindow.hpp"
#include "ViewerWidget.hpp"
#include "ArchiveContentsWidget.hpp"
#include <QMenuBar>
#include <QFileDialog>
#include <QApplication>
#include <QSettings>

ViewerWindow::ViewerWindow(QWidget* parent, Qt::WindowFlags flags): QMainWindow(parent, flags)
{
	setMinimumSize(640, 480);
	
	viewer = new ViewerWidget();
	this->setCentralWidget(viewer);
	
	archivewidget = new ArchiveContentsWidget;
	archivewidget->setObjectName("archivewidget");
	this->addDockWidget(Qt::LeftDockWidgetArea, archivewidget);
	
	QMenuBar* mb = this->menuBar();
	QMenu* file = mb->addMenu("&File");
	file->addAction("Open &Archive", this, SLOT(openArchive()));
	file->addSeparator();
	file->addAction("E&xit", QApplication::instance(), SLOT(quit()), QKeySequence::Quit);
	
	connect(archivewidget, SIGNAL(selectedFileChanged(std::string)), viewer, SLOT(showFile(std::string)));
	
	QSettings settings("OpenRW", "rwviewer");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
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
	
	archivewidget->setArchive(ld);
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
