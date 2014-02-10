#pragma once
#ifndef _VIEWERWIDGET_HPP_
#define _VIEWERWIDGET_HPP_
#include <QGLWidget>
#include <QTimer>

class ViewerWidget : public QGLWidget
{
	Q_OBJECT

	std::string currentFile;

	QTimer timer;

public:
    virtual void initializeGL();
	
    virtual void resizeGL(int w, int h);
	
    virtual void paintGL();

public slots:

	void showFile(const std::string& file);
};

#endif