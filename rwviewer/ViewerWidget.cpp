#include "ViewerWidget.hpp"

ViewerWidget::ViewerWidget(QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f)
: QGLWidget(parent, shareWidget, f), gworld(nullptr), fm(ViewerWidget::UNK)
{
}

void ViewerWidget::initializeGL()
{
	QGLWidget::initializeGL();
	timer.setInterval(16);
	connect(&timer, SIGNAL(timeout()), SLOT(updateGL()));
	timer.start();
	
	glewExperimental = 1;
	glewInit();
	
	gworld = new GameWorld("");
}

void ViewerWidget::resizeGL(int w, int h)
{
	QGLWidget::resizeGL(w, h);
	glViewport(0, 0, w, h);
}

void ViewerWidget::paintGL()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	if( !currentFile.isEmpty() ) {
		QString dbg = 
			QString("Viewing %1, %2")
				.arg(currentFile);
		switch(fm) { 
			case DFF: dbg = dbg.arg("DFF");
				break;
			case TXD: dbg = dbg.arg("TXD");
				break;
			case UNK: dbg = dbg.arg("Unknown");
				break;
		}
		renderText(5, height() - 5, dbg);
	}
}

GameWorld* ViewerWidget::world()
{
	return gworld;
}

void ViewerWidget::showFile(const QString& file)
{
	currentFile = file;
	QString low = file.toLower();
	if(low.endsWith("dff")) {
		showDFF(file);
	}
	else if(low.endsWith("txd")) {
		showTXD(file);
	}
	emit fileOpened(file);
}

void ViewerWidget::showDFF(const QString& file)
{
	gworld->gameData.loadDFF(file.toStdString());
	fm = ViewerWidget::DFF;
}

void ViewerWidget::showTXD(const QString& file)
{
	fm = ViewerWidget::TXD;
}
