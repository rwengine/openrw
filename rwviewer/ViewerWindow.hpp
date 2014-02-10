#pragma once 
#ifndef _VIEWERWINDOW_HPP_
#define _VIEWERWINDOW_HPP_
#include <QMainWindow>

class ViewerWidget;
class ViewerWindow : public QMainWindow
{
	ViewerWidget* viewer;
public:
	ViewerWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
};

#endif 