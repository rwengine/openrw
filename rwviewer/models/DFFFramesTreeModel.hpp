#pragma once
#ifndef _DFFFRAMESTREEMODEL_HPP_
#define _DFFFRAMESTREEMODEL_HPP_
#include <QAbstractItemModel>
#include <engine/RWTypes.hpp>

class Model;
class Skeleton;

class DFFFramesTreeModel : public QAbstractItemModel
{
	Model* model;
	Skeleton* skeleton;
public:
	
	explicit DFFFramesTreeModel(Model* m, Skeleton* skel, QObject* parent = 0);
	
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	
    virtual QModelIndex parent(const QModelIndex& child) const;
	
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

#endif
