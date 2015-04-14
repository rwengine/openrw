#include "ModelViewer.hpp"
#include <widgets/ModelFramesWidget.hpp>
#include "ViewerWidget.hpp"
#include <data/Skeleton.hpp>
#include <engine/GameObject.hpp>
#include <QDebug>

ModelViewer::ModelViewer(ViewerWidget* viewer, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f), _world(nullptr), viewing(nullptr), skeleton(nullptr)
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

	connect(frames, SIGNAL(selectedFrameChanged(ModelFrame*)),
			viewerWidget, SLOT(selectFrame(ModelFrame*)));
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
	if( skeleton )
	{
		delete skeleton;
	}
	skeleton = new Skeleton();
	viewerWidget->showModel(model);
	frames->setModel(model, nullptr);
}

void ModelViewer::showObject(uint16_t object)
{
	viewerWidget->showObject(object);
	viewing = viewerWidget->currentModel();
	skeleton = viewerWidget->currentObject()->skeleton;
	frames->setModel(viewing, skeleton);
}
