#include "ViewerWidget.hpp"

void ViewerWidget::initializeGL()
{
    QGLWidget::initializeGL();
}

void ViewerWidget::resizeGL(int w, int h)
{
    QGLWidget::resizeGL(w, h);
}

void ViewerWidget::paintGL()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	renderText(5, height() - 5, "Testing, 1 2 3 ...");
}
