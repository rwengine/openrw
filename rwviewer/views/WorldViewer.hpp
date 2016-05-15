#ifndef _RWVIEWER_WORLDVIEWER_HPP_
#define _RWVIEWER_WORLDVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>

#include "ViewerInterface.hpp"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLayout>

class WorldViewer : public ViewerInterface
{
	Q_OBJECT

	QVBoxLayout* mainLayout;
	ViewerWidget* viewerWidget;
public:
	WorldViewer(ViewerWidget *viewer = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);

	void setViewerWidget( ViewerWidget* widget ) override;

signals:
	void placementsLoaded(const QString& file);

public slots:
	void loadPlacements(const QString& file);
	void loadPlacements();
};

#endif
