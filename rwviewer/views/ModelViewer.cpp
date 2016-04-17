#include "ModelViewer.hpp"
#include <QDebug>
#include <widgets/ModelFramesWidget.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <objects/GameObject.hpp>
#include "ViewerWidget.hpp"

ModelViewer::ModelViewer(ViewerWidget* viewer, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f), _world(nullptr), viewing(nullptr), skeleton(nullptr)
{
	mainSplit = new QSplitter;
	mainLayout = new QVBoxLayout;

	viewerWidget = viewer;
	viewerWidget->setMinimumSize(250,250);

	animationWidget = new AnimationListWidget;
	connect(animationWidget, SIGNAL(selectedAnimationChanged(Animation*)),
			SLOT(playAnimation(Animation*)));

	frames = new ModelFramesWidget;
	frames->setMaximumWidth(300);

	mainSplit->addWidget(frames);
	mainSplit->addWidget(animationWidget);
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

void ModelViewer::loadAnimations(const QString& file)
{
	std::ifstream dfile(file.toStdString().c_str());
	AnimationList anims;

	if(dfile.is_open())
	{
		dfile.seekg(0, std::ios_base::end);
		size_t length = dfile.tellg();
		dfile.seekg(0);
		char *file = new char[length];
		dfile.read(file, length);

		LoaderIFP loader;
		if( loader.loadFromMemory(file) ) {
			for(auto& f : loader.animations) {
				anims.push_back(f);
			}
		}

		delete[] file;
	}

	animationWidget->setAnimations(anims);
}

void ModelViewer::playAnimation(Animation* anim)
{
	viewerWidget->currentObject()->animator->playAnimation(0, anim, 1.f, true);
}
