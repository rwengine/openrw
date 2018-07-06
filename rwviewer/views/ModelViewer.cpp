#include "ModelViewer.hpp"
#include <QDebug>
#include <engine/Animator.hpp>
#include <fstream>
#include <objects/GameObject.hpp>
#include <platform/FileHandle.hpp>
#include <widgets/ModelFramesWidget.hpp>
#include "ViewerWidget.hpp"

ModelViewer::ModelViewer(QWidget* parent, Qt::WindowFlags f)
    : ViewerInterface(parent, f), viewing(nullptr) {
    mainSplit = new QSplitter;
    mainLayout = new QVBoxLayout;

    frames = new ModelFramesWidget;
    frames->setMaximumWidth(300);

    viewerWidget = createViewer();
    viewerWidget->setMode(ViewerWidget::Mode::Model);

    mainSplit->addWidget(frames);
    mainSplit->addWidget(QWidget::createWindowContainer(viewerWidget));
    mainLayout->addWidget(mainSplit);

    setLayout(mainLayout);
}

void ModelViewer::showModel(ClumpPtr model) {
    viewing = model;
    viewerWidget->showModel(model);
    frames->setModel(model);
}

void ModelViewer::showObject(uint16_t object) {
    auto def = world()->data->modelinfo[object].get();
    if (!def) {
        return;
    }

    auto modelName = def->name + ".dff";
    auto textureName = def->name + ".txd";
    auto textures = world()->data->loadTextureArchive(textureName);

    LoaderDFF dffLoader;
    dffLoader.setTextureLookupCallback(
        [&](const std::string& texture, const std::string&) {
            return textures.at(texture);
        });

    auto file = world()->data->index.openFile(modelName);
    if (!file.data) {
        RW_ERROR("Couldn't load " << modelName);
        return;
    }
    showModel(dffLoader.loadFromMemory(file));
}


