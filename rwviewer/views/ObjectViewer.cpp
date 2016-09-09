#include "ObjectViewer.hpp"
#include <QDebug>
#include <QMenu>
#include <models/ObjectListModel.hpp>
#include "ViewerWidget.hpp"

ObjectViewer::ObjectViewer(ViewerWidget* viewer, QWidget* parent,
                           Qt::WindowFlags f)
    : ViewerInterface(parent, f) {
    mainLayout = new QHBoxLayout;

    objectList = new QTableView;

    objectMenu = new QMenu(objectList);
    objectList->setContextMenuPolicy(Qt::CustomContextMenu);
    auto viewModelAction = new QAction("View Model", objectMenu);
    objectMenu->addAction(viewModelAction);
    connect(viewModelAction, SIGNAL(triggered()), this, SLOT(menuViewModel()));
    connect(objectList, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(onCustomContextMenu(QPoint)));

    mainLayout->addWidget(objectList);

    previewWidget = viewer;
    previewWidget->setMinimumSize(250, 250);

    infoLayout = new QGridLayout;

    previewID = new QLabel;
    previewModel = new QLabel;
    previewClass = new QLabel;
    infoLayout->addWidget(new QLabel("ID"), 1, 0);
    infoLayout->addWidget(previewID, 1, 1);
    infoLayout->addWidget(new QLabel("Type"), 2, 0);
    infoLayout->addWidget(previewClass, 2, 1);
    infoLayout->addWidget(new QLabel("Model"), 3, 0);
    infoLayout->addWidget(previewModel, 3, 1);

    mainLayout->addLayout(infoLayout);

    this->setLayout(mainLayout);

    setViewerWidget(previewWidget);
}

void ObjectViewer::setViewerWidget(ViewerWidget* widget) {
    // widgetLayout->removeWidget(previewWidget);
    previewWidget = widget;
    infoLayout->addWidget(previewWidget, 0, 0, 1, 2);
}

void ObjectViewer::worldChanged() {
    if (objectList->model()) {
        delete objectList->model();
    }

    objectList->setModel(new ObjectListModel(world()->data, objectList));
    connect(objectList->selectionModel(),
            SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
            SLOT(showItem(QModelIndex)));
}

static std::map<ObjectInformation::ObjectClass, QString> gDataType = {
    {ObjectInformation::_class("OBJS"), "Object"},
    {ObjectInformation::_class("CARS"), "Vehicle"},
    {ObjectInformation::_class("PEDS"), "Pedestrian"},
    {ObjectInformation::_class("HIER"), "Cutscene"}};

void ObjectViewer::showItem(qint16 item) {
    auto def = world()->data->objectTypes[item];

    if (def) {
        previewID->setText(QString::number(def->ID));
        previewClass->setText(gDataType[def->class_type]);

        if (def->class_type == ObjectData::class_id) {
            auto v = std::static_pointer_cast<ObjectData>(def);
            previewModel->setText(QString::fromStdString(v->modelName));
        } else if (def->class_type == VehicleData::class_id) {
            auto v = std::static_pointer_cast<VehicleData>(def);
            previewModel->setText(QString::fromStdString(v->modelName));
        } else if (def->class_type == CharacterData::class_id) {
            auto v = std::static_pointer_cast<CharacterData>(def);
            previewModel->setText(QString::fromStdString(v->modelName));
        }

        previewWidget->showObject(item);
    }
}

void ObjectViewer::showItem(QModelIndex model) {
    showItem(model.internalId());
}

void ObjectViewer::onCustomContextMenu(const QPoint& p) {
    contextMenuIndex = objectList->indexAt(p);
    if (contextMenuIndex.isValid()) {
        objectMenu->exec(objectList->mapToGlobal(p));
    }
}

void ObjectViewer::menuViewModel() {
    if (contextMenuIndex.isValid()) {
        auto id = contextMenuIndex.internalId();
        showObjectModel(id);
    }
}
