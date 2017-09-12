#include "ModelFramesWidget.hpp"
#include <data/Clump.hpp>
#include <glm/gtx/string_cast.hpp>

void ModelFramesWidget::updateInfoBox(ClumpPtr model, ModelFrame* f) {
    if (f == nullptr) {
        _frameLabel->setText("");
    } else {
        auto labText = QString("Name: %1\nTranslation: %2")
                           .arg(QString::fromStdString(f->getName()))
                           .arg(QString::fromStdString(
                               glm::to_string(f->getDefaultTranslation())));
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

void ModelFramesWidget::setModel(ClumpPtr model) {
    if (framemodel) {
        delete framemodel;
        framemodel = nullptr;
        tree->setModel(nullptr);
    }
    gmodel = model;
    if (model.get() != nullptr) {
        framemodel = new DFFFramesTreeModel(model, this);
        tree->setModel(framemodel);
        tree->setDisabled(false);
        connect(tree->selectionModel(),
                SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                SLOT(selectedModelChanged(QModelIndex, QModelIndex)));
    } else {
        tree->setDisabled(true);
    }
}
