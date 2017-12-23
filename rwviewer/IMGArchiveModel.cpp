#include "IMGArchiveModel.hpp"

QVariant IMGArchiveModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        if (index.row() >= 0 &&
            (unsigned)index.row() < archive.getAssetCount()) {
            auto& f = archive.getAssetInfoByIndex(index.row());
            if (index.column() == 0) {
                return QString(f.name);
            }
            if (index.column() == 1) {
                return f.size;
            }
        }
    }
    return QVariant::Invalid;
}

QVariant IMGArchiveModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return "Name";
        }
        if (section == 1) {
            return "Size";
        }
    }
    return QVariant::Invalid;
}

int IMGArchiveModel::rowCount(const QModelIndex&) const {
    return archive.getAssetCount();
}

int IMGArchiveModel::columnCount(const QModelIndex&) const {
    return 2;
}
