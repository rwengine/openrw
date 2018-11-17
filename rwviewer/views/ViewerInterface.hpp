#ifndef _RWVIEWER_VIEWERINTERFACE_HPP_
#define _RWVIEWER_VIEWERINTERFACE_HPP_
#include <engine/GameWorld.hpp>

#include <ViewerWidget.hpp>

#include <QWidget>

class ViewerInterface : public QWidget {
    Q_OBJECT
public:
    ViewerInterface(QWidget* parent = 0, Qt::WindowFlags f = 0)
        : QWidget(parent, f), m_world(nullptr) {
    }

    GameWorld* world() {
        return m_world;
    }

protected:
    virtual void worldChanged() {
    }

    ViewerWidget* createViewer();

signals:
    void gameLoaded(GameWorld*, GameRenderer*);

public slots:
    void showData(GameWorld* world, GameRenderer* renderer) {
        m_world = world;
        gameLoaded(world, renderer);
        worldChanged();
    }

private:
    GameWorld* m_world;
};

#endif
