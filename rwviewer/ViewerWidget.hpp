#pragma once
#ifndef _VIEWERWIDGET_HPP_
#define _VIEWERWIDGET_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <QGLWidget>
#include <QTimer>
#include <loaders/LoaderIFP.hpp>

class Model;
class ViewerWidget : public QGLWidget
{
	Q_OBJECT

	QString currentFile;

	QTimer timer;
	GameWorld* gworld;

	GameObject* dummyObject;
	quint16 currentObjectID;
	
	ModelHandle* cmodel;
	Animation* canimation;
	
	float viewDistance;
	glm::vec2 viewAngles;
	
	bool dragging;
	QPointF dstart;
	glm::vec2 dastart;
public:

    ViewerWidget(QWidget* parent = 0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f = 0);

    virtual void initializeGL();
	
    virtual void resizeGL(int w, int h);
	
    virtual void paintGL();

	ModelHandle *currentModel() const;
	
	// TODO: Move this into the main window or elsewhere, doesn't really belong here.
	void setGamePath(const std::string& path);
	GameWorld* world();

public slots:

	void showItem(qint16 item);

	void showAnimation(Animation* anim);

	void exportModel();

signals:

	void dataLoaded(GameWorld* world);

	void fileOpened(const QString& file);

protected:
	
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent*);
};

#endif
