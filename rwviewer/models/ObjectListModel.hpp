#ifndef _OBJECTLISTMODEL_HPP_
#define _OBJECTLISTMODEL_HPP_

#include <QAbstractItemModel>

#include <engine/GameWorld.hpp>

class ObjectListModel : public QAbstractTableModel
{
	Q_OBJECT

	GameWorld* _world;

public:
	explicit ObjectListModel(GameWorld* _world, QObject *parent = 0);

	GameWorld* world() const { return _world; }

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
};

#endif // _OBJECTLISTMODEL_HPP_
