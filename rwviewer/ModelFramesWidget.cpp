#include "ModelFramesWidget.hpp"

ModelFramesWidget::ModelFramesWidget(QWidget* parent, Qt::WindowFlags flags)
: QDockWidget(parent, flags), gmodel(nullptr), framemodel(nullptr)
{
	setWindowTitle("Frames");
	tree = new QTreeView;
	setWidget(tree);
}

void ModelFramesWidget::setModel(Model *model)
{
	if(framemodel) {
		delete framemodel;
		framemodel = nullptr;
		tree->setModel(nullptr);
	}
	gmodel = model;
	if(model != nullptr) {
		framemodel = new DFFFramesTreeModel(model, this);
		tree->setModel(framemodel);
		tree->setDisabled(false);
	}
	else {
		tree->setDisabled(true);
	}
}
