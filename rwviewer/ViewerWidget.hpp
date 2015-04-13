#pragma once
#ifndef _VIEWERWIDGET_HPP_
#define _VIEWERWIDGET_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <QGLWidget>
#include <QTimer>
#include <loaders/LoaderIFP.hpp>
#include <render/DrawBuffer.hpp>
#include <render/GeometryBuffer.hpp>
#include <render/Model.hpp>
#include <glm/glm.hpp>

class GameRenderer;
class Model;
class ViewerWidget : public QGLWidget
{
	Q_OBJECT

	GameRenderer* renderer;

	QString currentFile;

	QTimer timer;
	GameWorld* gworld;

	Model* activeModel;
	GameObject* dummyObject;
	quint16 currentObjectID;
	
	Model* _lastModel;
	Animation* canimation;
	
	float viewDistance;
	glm::vec2 viewAngles;
	
	bool dragging;
	QPointF dstart;
	glm::vec2 dastart;
	
	DrawBuffer* _frameWidgetDraw;
	GeometryBuffer* _frameWidgetGeom;
	void drawFrameWidget(ModelFrame* f, const glm::mat4& = glm::mat4(1.f));
public:

	ViewerWidget(QWidget* parent = 0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f = 0);

	virtual void initializeGL();
	
	virtual void resizeGL(int w, int h);
	
	virtual void paintGL();

	Model *currentModel() const;

	GameWorld* world();

public slots:

	void showItem(qint16 item);
	void showModel(Model* model);

	void exportModel();

	void dataLoaded(GameWorld* world);

	void setRenderer(GameRenderer* renderer);

signals:

	void fileOpened(const QString& file);

	void modelChanged(Model* model);

protected:
	
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent*);
};

#endif
