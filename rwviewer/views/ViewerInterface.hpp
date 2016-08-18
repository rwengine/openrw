#ifndef _RWVIEWER_VIEWERINTERFACE_HPP_
#define _RWVIEWER_VIEWERINTERFACE_HPP_
#include <engine/GameWorld.hpp>

#include <ViewerWidget.hpp>

#include <QWidget>

class ViewerInterface : public QWidget
{
  Q_OBJECT
public:
  ViewerInterface(QWidget* parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f), m_world(nullptr)
  {
  }

  virtual void setViewerWidget(ViewerWidget* widget) = 0;

  GameWorld* world() { return m_world; }

protected:
  virtual void worldChanged() {}

public slots:
  void showData(GameWorld* world)
  {
    m_world = world;
    worldChanged();
  }

private:
  GameWorld* m_world;
};

#endif
