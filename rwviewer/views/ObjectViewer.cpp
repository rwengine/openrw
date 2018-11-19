#include "ObjectViewer.hpp"
#include <ViewerWindow.hpp>
#include <models/ObjectListModel.hpp>

#include <QCheckBox>
#include <QLineEdit>
#include <QMenu>


class ObjectSearchModel : public QSortFilterProxyModel {
public:
    explicit ObjectSearchModel(QObject* parent)
        : QSortFilterProxyModel(parent) {
    }

    void showCARS(bool cars) {
        _showCars = cars;
        invalidateFilter();
    }

    void showOBJS(bool objs) {
        _showMisc = objs;
        invalidateFilter();
    }

    void showPEDS(bool peds) {
        _showPeds = peds;
        invalidateFilter();
    }

    void filterName(const QString& name) {
        _name = name.toStdString();
        invalidateFilter();
    }

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override
    {
        auto index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto& model = _data->modelinfo.at(index0.internalId());

        switch (model->type()) {
            case ModelDataType::VehicleInfo:
                if (!_showCars) {
                    return false;
                }
                break;
            case ModelDataType::PedInfo:
                if (!_showPeds) {
                    return false;
                }
                break;
            default:
                if (!_showMisc) {
                    return false;
                }
                break;
        }

        return !(!_name.empty() && model->name.find(_name) == std::string::npos);
    }

    void setModel(ObjectListModel* model) {
        _data = model->gameData();
        QSortFilterProxyModel::setSourceModel(model);
    }

    GameData* _data = nullptr;
private:
    bool _showPeds = true;
    bool _showMisc = true;
    bool _showCars = true;

    std::string _name;
};

namespace {
QLayout* searchControls(ObjectSearchModel* search) {
    auto bar = new QHBoxLayout;

    auto searchBox = new QLineEdit;
    searchBox->setPlaceholderText("Search");
    QObject::connect(searchBox, &QLineEdit::textChanged, search, &ObjectSearchModel::filterName);

    auto cars = new QCheckBox;
    cars->setText("CARS");
    cars->setChecked(true);
    QObject::connect(cars, &QCheckBox::clicked, search, &ObjectSearchModel::showCARS );
    auto peds = new QCheckBox;
    peds->setText("PEDS");
    peds->setChecked(true);
    QObject::connect(peds, &QCheckBox::clicked, search, &ObjectSearchModel::showPEDS );
    auto misc = new QCheckBox;
    misc->setText("Misc");
    misc->setChecked(true);
    QObject::connect(misc, &QCheckBox::clicked, search, &ObjectSearchModel::showOBJS );

    bar->addWidget(searchBox, 1);
    bar->addWidget(cars);
    bar->addWidget(peds);
    bar->addWidget(misc);

    return bar;
}
}  // namespace

ObjectViewer::ObjectViewer(QWidget* parent, Qt::WindowFlags f)
    : ViewerInterface(parent, f) {
    mainLayout = new QHBoxLayout(this);

    auto leftLayout = new QVBoxLayout;

    objectList = new QTableView;
    objectMenu = new QMenu(objectList);
    objectList->setContextMenuPolicy(Qt::CustomContextMenu);
    auto viewModelAction = new QAction("View Model", objectMenu);
    objectMenu->addAction(viewModelAction);
    connect(viewModelAction, SIGNAL(triggered()), this, SLOT(menuViewModel()));
    connect(objectList, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(onCustomContextMenu(QPoint)));
    filterModel = new ObjectSearchModel(this);
    objectList->setModel(filterModel);
    objectList->setColumnWidth(0, 50);
    objectList->setColumnWidth(1, 150);
    objectList->setColumnWidth(2, 200);
    objectList->setSortingEnabled(true);
    objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    connect(objectList->selectionModel(),
            SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
            SLOT(showItem(QModelIndex)));

    leftLayout->addLayout(searchControls(filterModel));
    leftLayout->addWidget(objectList);
    mainLayout->addLayout(leftLayout, 6);

    previewWidget = createViewer();
    previewWidget->setMode(ViewerWidget::Mode::Object);

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
    infoLayout->addWidget(QWidget::createWindowContainer(previewWidget), 0, 0, 1, 2);
    infoLayout->setRowStretch(0, 1);

    mainLayout->addLayout(infoLayout, 4);
    setLayout(mainLayout);
}

void ObjectViewer::worldChanged() {
    if (filterModel->sourceModel()) {
        delete filterModel->sourceModel();
    }

    auto newModel = new ObjectListModel(world()->data, this);
    filterModel->setModel(newModel);
    objectList->sortByColumn(0, Qt::AscendingOrder);

    objectList->resizeColumnsToContents();
}

void ObjectViewer::showItem(qint16 item) {
    auto def = world()->data->modelinfo[item].get();

    if (def) {
        previewID->setText(QString::number(def->id()));
        previewClass->setText(
            QString::fromStdString(BaseModelInfo::getTypeName(def->type())));
        previewModel->setText(QString::fromStdString(def->name));
        previewWidget->showObject(item);
    }
}

void ObjectViewer::showItem(QModelIndex model) {
    auto source = filterModel->mapToSource(model);
    showItem(source.internalId());
}

void ObjectViewer::onCustomContextMenu(const QPoint& p) {
    contextMenuIndex = objectList->indexAt(p);
    if (contextMenuIndex.isValid()) {
        objectMenu->exec(objectList->mapToGlobal(p));
    }
}

void ObjectViewer::menuViewModel() {
    if (contextMenuIndex.isValid()) {
        auto source = filterModel->mapToSource(contextMenuIndex);
        showObjectModel(source.internalId());
    }
}
