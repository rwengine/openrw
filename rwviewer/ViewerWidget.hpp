#pragma once
#ifndef _VIEWERWIDGET_HPP_
#define _VIEWERWIDGET_HPP_
#include <QTimer>
#include <data/Clump.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>
#include <glm/glm.hpp>
#include <loaders/LoaderIFP.hpp>

// Prevent Qt from conflicting with glLoadGen
#define GL_ARB_debug_output
#define GL_KHR_debug
#include <QGLWidget>

class GameRenderer;
class Clump;
class ViewerWidget : public QGLWidget {
    Q_OBJECT

    GameRenderer* renderer;

    QString currentFile;

    QTimer timer;
    GameWorld* gworld;

    ClumpPtr activeModel;
    ModelFrame* selectedFrame;
    GameObject* dummyObject;
    quint16 currentObjectID;

    ClumpPtr _lastModel;
    Animation* canimation;

    float viewDistance;
    glm::vec2 viewAngles;
    glm::vec3 viewPosition;

    bool dragging;
    QPointF dstart;
    glm::vec2 dastart;
    bool moveFast;

    DrawBuffer* _frameWidgetDraw;
    GeometryBuffer* _frameWidgetGeom;
    GLuint whiteTex;

    void drawFrameWidget(ModelFrame* f, const glm::mat4& = glm::mat4(1.f));

public:
    ViewerWidget(QGLFormat g, QWidget* parent = 0,
                 const QGLWidget* shareWidget = 0, Qt::WindowFlags f = 0);

    virtual void initializeGL();

    virtual void resizeGL(int w, int h);

    virtual void paintGL();

    ClumpPtr currentModel() const;
    GameObject* currentObject() const;

    GameWorld* world();

public slots:

    void showObject(qint16 item);
    void showModel(ClumpPtr model);
    void selectFrame(ModelFrame* frame);

    void exportModel();

    void dataLoaded(GameWorld* world);

    void setRenderer(GameRenderer* renderer);

signals:

    void fileOpened(const QString& file);

    void modelChanged(ClumpPtr model);

protected:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
};

#endif
