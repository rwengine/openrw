#include "ViewerWindow.hpp"
#include "ViewerWidget.hpp"
#include "ArchiveContentsWidget.hpp"
#include <QMenuBar>
#include <QFileDialog>

ViewerWindow::ViewerWindow(QWidget* parent, Qt::WindowFlags flags): QMainWindow(parent, flags)
{
	setMinimumSize(640, 480);
	
	viewer = new ViewerWidget();
	this->setCentralWidget(viewer);
	
	archivewidget = new ArchiveContentsWidget;
	this->addDockWidget(Qt::LeftDockWidgetArea, archivewidget);
	
	QMenuBar* mb = this->menuBar();
	QMenu* file = mb->addMenu("&File");
	file->addAction("Open &Archive", this, SLOT(openArchive()));
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

void ViewerWindow::openArchive()
{
	QFileDialog dialog(this, "Open Archive", QDir::homePath(), "IMG Archives (*.img)");
	if(dialog.exec()) {
		openArchive(dialog.selectedFiles().at(0));
	}
}
