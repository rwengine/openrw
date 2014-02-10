#pragma once
#ifndef _VIEWERWIDGET_HPP_
#define _VIEWERWIDGET_HPP_
#include <QGLWidget>

class ViewerWidget : public QGLWidget
{
public:
    virtual void initializeGL();
	
    virtual void resizeGL(int w, int h);
	
    virtual void paintGL();
};

#endif