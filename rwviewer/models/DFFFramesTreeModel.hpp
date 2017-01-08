#pragma once
#ifndef _DFFFRAMESTREEMODEL_HPP_
#define _DFFFRAMESTREEMODEL_HPP_
#include <QAbstractItemModel>
#include <rw/types.hpp>

class Clump;

class DFFFramesTreeModel : public QAbstractItemModel {
    Clump* model;

public:
    explicit DFFFramesTreeModel(Clump* m, QObject* parent = 0);

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex& index, const QVariant& value,
                         int role);

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex parent(const QModelIndex& child) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
};

#endif
