#include <QApplication>
#include <QStyleFactory>
#include "ViewerWindow.hpp"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	/* HACK until Qt curve supports Qt5 */
	QApplication::setStyle(QStyleFactory::create("GTK+"));
	
	ViewerWindow viewer;
	viewer.show();
	
	return app.exec();
}
