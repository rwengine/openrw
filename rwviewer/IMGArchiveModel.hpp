#pragma once 
#ifndef _IMGARCHIVEMODEL_HPP_
#define _IMGARCHIVEMODEL_HPP_
#include <QAbstractItemModel>
#include <loaders/LoaderIMG.hpp>

class IMGArchiveModel : public QAbstractListModel
{
	Q_OBJECT
	
	LoaderIMG archive;
	
public:
	
    IMGArchiveModel(const LoaderIMG& archive, QObject* parent = 0)
		: QAbstractListModel(parent), archive(archive)
		{}

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

#endif 