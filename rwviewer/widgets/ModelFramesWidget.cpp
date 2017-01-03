#include "ModelFramesWidget.hpp"
#include <data/Clump.hpp>
#include <glm/gtx/string_cast.hpp>

void ModelFramesWidget::updateInfoBox(Clump* model, ModelFrame* f) {
    if (f == nullptr) {
        _frameLabel->setText("");
    } else {
        auto labText = QString("Name: %1\nTranslation: %2\nTextures:%3")
                           .arg(QString::fromStdString(f->getName()))
                           .arg(QString::fromStdString(
                               glm::to_string(f->getDefaultTranslation())));
        QString geomString;
        for (size_t gi : f->getGeometries()) {
            auto& g = model->geometries[gi];
            // for(Model::SubGeometry& sg : g->subgeom)
            for (Geometry::Material& m : g->materials) {
                for (Geometry::Texture& t : m.textures) {
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

void ModelFramesWidget::selectedModelChanged(const QModelIndex& n,
                                             const QModelIndex&) {
    updateInfoBox(gmodel, (ModelFrame*)n.internalPointer());
    selectedFrameChanged((ModelFrame*)n.internalPointer());
}

ModelFramesWidget::ModelFramesWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags), gmodel(nullptr), framemodel(nullptr) {
    setWindowTitle("Frames");

    _layout = new QVBoxLayout;
    tree = new QTreeView(this);
    _layout->addWidget(tree);
    _frameLabel = new QLabel(this);
    _layout->addWidget(_frameLabel);

    setLayout(_layout);
}

void ModelFramesWidget::setModel(Clump* model, Skeleton* skeleton) {
    if (framemodel) {
        delete framemodel;
        framemodel = nullptr;
        tree->setModel(nullptr);
    }
    gmodel = model;
    if (model != nullptr) {
        framemodel = new DFFFramesTreeModel(model, skeleton, this);
        tree->setModel(framemodel);
        tree->setDisabled(false);
        connect(tree->selectionModel(),
                SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                SLOT(selectedModelChanged(QModelIndex, QModelIndex)));
    } else {
        tree->setDisabled(true);
    }
}
