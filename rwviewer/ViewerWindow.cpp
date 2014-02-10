#include "ViewerWindow.hpp"
#include "ViewerWidget.hpp"

ViewerWindow::ViewerWindow(QWidget* parent, Qt::WindowFlags flags): QMainWindow(parent, flags)
{
	viewer = new ViewerWidget();
	this->setCentralWidget(viewer);
}
