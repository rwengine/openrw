#pragma once 
#ifndef _IMGTABLEMODEL_HPP_
#define _IMGTABLEMODEL_HPP_
#include <QAbstractItemModel>
#include <loaders/LoaderIMG.hpp>

class IMGTableModel : public QAbstractTableModel
{
	Q_OBJECT
	
	LoaderIMG archive;
	
public:
	
    IMGTableModel(const LoaderIMG& archive, QObject* parent = 0)
		: QAbstractTableModel(parent), archive(archive)
		{}

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

#endif 