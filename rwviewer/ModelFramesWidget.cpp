#include "ModelFramesWidget.hpp"
#include <render/Model.hpp>
#include <glm/gtx/string_cast.hpp>

void ModelFramesWidget::updateInfoBox(Model* model, ModelFrame *f)
{
	if( f == nullptr ) {
		_frameLabel->setText("");
	}
	else {
		auto labText = QString("Name: %1\nTranslation: %2\nTextures:%3")
				.arg(QString::fromStdString(f->getName()))
				.arg(QString::fromStdString(glm::to_string(f->getDefaultTranslation())));
		QString geomString;
		for(size_t gi : f->getGeometries()) {
			auto& g = model->geometries[gi];
			//for(Model::SubGeometry& sg : g->subgeom)
			for(Model::Material& m : g->materials) {
				for(Model::Texture& t : m.textures) {
					geomString += QString("\n %1 (%2)")
							.arg(t.name.c_str())
							.arg(t.alphaName.c_str());
				}
			}
		}
		labText = labText.arg(geomString);
		_frameLabel->setText(labText);
	}
}

void ModelFramesWidget::selectedModelChanged(const QModelIndex & n, const QModelIndex &)
{
	updateInfoBox( gmodel, (ModelFrame*) n.internalPointer() );
}

ModelFramesWidget::ModelFramesWidget(QWidget* parent, Qt::WindowFlags flags)
: QDockWidget(parent, flags), gmodel(nullptr), framemodel(nullptr)
{
	setWindowTitle("Frames");

	QWidget* w = new QWidget(this);

	_layout = new QVBoxLayout(w);
	tree = new QTreeView(this);
	_layout->addWidget(tree);
	_frameLabel = new QLabel(this);
	_layout->addWidget(_frameLabel);

	setWidget(w);
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
		connect(tree->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
				SLOT(selectedModelChanged(QModelIndex,QModelIndex)));
	}
	else {
		tree->setDisabled(true);
	}
}
