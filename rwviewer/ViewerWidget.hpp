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
	
	Model* cmodel;
	Animation* canimation;
	
	float viewDistance;
	glm::vec2 viewAngles;
	
	bool dragging;
	QPointF dstart;
	glm::vec2 dastart;
public:
	
	enum FileMode {
		TXD, DFF, UNK
	};
	
    ViewerWidget(QWidget* parent = 0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f = 0);

    virtual void initializeGL();
	
    virtual void resizeGL(int w, int h);
	
    virtual void paintGL();
	
	FileMode fileMode() const;
	
	Model* currentModel() const;
	
	GameWorld* world();

public slots:

	void showFile(const QString& file);
	void showDFF(const QString& file);
	void showTXD(const QString& file);

	void showAnimation(Animation* anim);

signals:

	void fileOpened(const QString& file);

protected:
	
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent*);

private:
	FileMode fm;
};

#endif
