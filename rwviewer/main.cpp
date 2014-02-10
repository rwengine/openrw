#include <QApplication>
#include "ViewerWindow.hpp"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	ViewerWindow viewer;
	viewer.show();
	
	return app.exec();
}
