#include "ItemListModel.hpp"
#include <engine/GameData.hpp>

qint16 ItemListModel::getIDOf(unsigned int row) const {
    if (row < world()->data->modelinfo.size()) {
        return row;
    }

    return -1;
}

ItemListModel::ItemListModel(GameWorld* world, QObject* parent)
    : QAbstractTableModel(parent), _world(world) {
}

int ItemListModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(_world->data->modelinfo.size());
}

int ItemListModel::columnCount(const QModelIndex&) const {
    return 2;
}

QVariant ItemListModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        qint16 id = getIDOf(index.row());
        if (id == -1)
            return QVariant::Invalid;
        if (index.column() == 0) {
            return id;
        } else if (index.column() == 1) {
            return QString::fromStdString("TODO");
        }
    }
    return QVariant::Invalid;
}

QVariant ItemListModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return "ID";
        }
        if (section == 1) {
            return "Model";
        }
    }
    return QVariant::Invalid;
}

QModelIndex ItemListModel::index(int row, int column,
                                 const QModelIndex& parent) const {
    return hasIndex(row, column, parent)
               ? createIndex(row, column, getIDOf(row))
               : QModelIndex();
}
