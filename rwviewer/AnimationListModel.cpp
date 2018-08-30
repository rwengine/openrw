#include "AnimationListModel.hpp"

QVariant AnimationListModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        if (index.row() >= 0 &&
            static_cast<unsigned>(index.row()) < animations.size()) {
            auto& f = animations.at(index.row());
            if (index.column() == 0) {
                return QString(f.first.c_str());
            }
        }
    }
    return QVariant::Invalid;
}

QVariant AnimationListModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return "Name";
        }
    }
    return QVariant::Invalid;
}

int AnimationListModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(animations.size());
}

int AnimationListModel::columnCount(const QModelIndex&) const {
    return 1;
}
