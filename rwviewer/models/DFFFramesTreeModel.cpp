#include "DFFFramesTreeModel.hpp"
#include <data/Clump.hpp>
#include <utility>

DFFFramesTreeModel::DFFFramesTreeModel(ClumpPtr m, QObject* parent)
    : QAbstractItemModel(parent), model(std::move(std::move(m))) {
}

int DFFFramesTreeModel::columnCount(const QModelIndex& /*parent*/) const {
    return 1;
}

int DFFFramesTreeModel::rowCount(const QModelIndex& parent) const {
    auto f = static_cast<ModelFrame*>(parent.internalPointer());
    if (f) {
        return static_cast<int>(f->getChildren().size());
    }

    if (parent.row() == -1) {
        return 1;
    }

    return 0;
}

QModelIndex DFFFramesTreeModel::index(int row, int column,
                                      const QModelIndex& parent) const {
    if (parent.row() == -1 && parent.column() == -1) {
        return createIndex(row, column, model->getFrame().get());
    }
    auto f = static_cast<ModelFrame*>(parent.internalPointer());
    ModelFrame* p = f->getChildren()[row].get();
    return createIndex(row, column, p);
}

QModelIndex DFFFramesTreeModel::parent(const QModelIndex& child) const {
    auto c = static_cast<ModelFrame*>(child.internalPointer());
    if (c->getParent()) {
        auto cp = c->getParent();
        if (cp->getParent() != nullptr) {
            for (size_t i = 0; i < cp->getParent()->getChildren().size(); ++i) {
                if (cp->getParent()->getChildren()[i].get() == c->getParent()) {
                    return createIndex(static_cast<int>(i), 0, c->getParent());
                }
            }
        } else {
            return createIndex(0, 0, cp);
        }
    }
    return QModelIndex();
}

QVariant DFFFramesTreeModel::data(const QModelIndex& index, int role) const {
    auto f = static_cast<ModelFrame*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return QString(f->getName().c_str());
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == 0) {
            return true;
        }
    }
    return QVariant();
}

bool DFFFramesTreeModel::setData(const QModelIndex& index,
                                 const QVariant& value, int role) {
    if (!index.isValid()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        if (index.column() == 0) {
            if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked) {
                RW_UNIMPLEMENTED("Hiding Frames");
            } else {
            }
            return true;
        }
    }

    return false;
}

Qt::ItemFlags DFFFramesTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return nullptr;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

QVariant DFFFramesTreeModel::headerData(int /*section*/,
                                        Qt::Orientation orientation,
                                        int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return "Name";
        }
    }
    return QVariant();
}
