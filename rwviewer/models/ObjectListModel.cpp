#include "ObjectListModel.hpp"

ObjectListModel::ObjectListModel(GameData *gameData, QObject *parent)
    : QAbstractTableModel(parent), _gameData(gameData) {
}

int ObjectListModel::rowCount(const QModelIndex & /*parent*/) const {
    return static_cast<int>(_gameData->modelinfo.size());
}

int ObjectListModel::columnCount(const QModelIndex & /*parent*/) const {
    return 3;
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        if (!index.isValid()) {
            return QVariant::Invalid;
        }
        auto id = index.internalId();
        if (index.column() == 0) {
            return id;
        }
        if (index.column() == 1) {
            auto object = _gameData->modelinfo[id].get();
            return QString::fromStdString(
                BaseModelInfo::getTypeName(object->type()));
        }
        if (index.column() == 2) {
            auto object = _gameData->modelinfo[id].get();
            return QString::fromStdString(object->name);
        }
    }
    return QVariant::Invalid;
}

QVariant ObjectListModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return "ID";
            case 1:
                return "Type";
            case 2:
                return "Model";
        }
    }
    return QVariant::Invalid;
}

QModelIndex ObjectListModel::index(int row, int column,
                                   const QModelIndex &parent) const {
    auto it = _gameData->modelinfo.begin();
    for (int i = 0; i < row; i++) {
        ++it;
    }
    auto id = it->second->id();

    return hasIndex(row, column, parent) ? createIndex(row, column, id)
                                         : QModelIndex();
}
