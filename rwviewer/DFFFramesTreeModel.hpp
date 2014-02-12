#pragma once
#ifndef _DFFFRAMESTREEMODEL_HPP_
#define _DFFFRAMESTREEMODEL_HPP_
#include <QAbstractItemModel>

class Model;
class DFFFramesTreeModel : public QAbstractItemModel
{
	Model* model;
public:
	
    explicit DFFFramesTreeModel(Model* m, QObject* parent = 0);
	
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	
    virtual QModelIndex parent(const QModelIndex& child) const;
	
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

#endif