#include "ModelViewer.hpp"
#include <widgets/ModelFramesWidget.hpp>
#include "ViewerWidget.hpp"
#include <QDebug>

ModelViewer::ModelViewer(ViewerWidget* viewer, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f), _world(nullptr), viewing(nullptr)
{
	mainSplit = new QSplitter;
	mainLayout = new QVBoxLayout;

	viewerWidget = viewer;
	viewerWidget->setMinimumSize(250,250);

	frames = new ModelFramesWidget;
	frames->setMaximumWidth(300);

	mainSplit->addWidget(frames);
	mainLayout->addWidget(mainSplit);

	this->setLayout(mainLayout);

	setViewerWidget(viewerWidget);
}

void ModelViewer::setViewerWidget(ViewerWidget* widget)
{
	viewerWidget = widget;
	mainSplit->addWidget(viewerWidget);
	showModel(viewing);
}

void ModelViewer::showData(GameWorld* world)
{
	_world = world;
}

void ModelViewer::showModel(Model* model)
{
	viewing = model;
	viewerWidget->showModel(model);
	frames->setModel(model);
}
