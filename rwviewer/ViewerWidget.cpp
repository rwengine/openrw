#include "ViewerWidget.hpp"

void ViewerWidget::initializeGL()
{
	QGLWidget::initializeGL();
	timer.setInterval(16);
	connect(&timer, SIGNAL(timeout()), SLOT(updateGL()));
	timer.start();
}

void ViewerWidget::resizeGL(int w, int h)
{
	QGLWidget::resizeGL(w, h);
}

void ViewerWidget::paintGL()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	if( !currentFile.empty() ) {
		QString dbg;
		dbg.append("Viewing: ");
		dbg.append(currentFile.c_str());
		renderText(5, height() - 5, dbg);
	}
}

void ViewerWidget::showFile(const std::string& file)
{
	currentFile = file;
}