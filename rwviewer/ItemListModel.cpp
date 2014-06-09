#include "ItemListModel.hpp"

qint16 ItemListModel::getIDOf(unsigned int row) const
{
	unsigned int j = 0;
	if( row < world()->objectTypes.size() ) {
		auto it = world()->objectTypes.begin();
		while (j < row) {
			it++; j++;
			if( it == world()->objectTypes.end() ) return -1;
		}
		return it->first;
	}
	return -1;

	row -= world()->objectTypes.size();
	if( row < world()->vehicleTypes.size() ) {
		auto it = world()->vehicleTypes.begin();
		while (j < row) {
			it++; j++;
			if( it == world()->vehicleTypes.end() ) return -1;
		}
		return it->first;
	}
	return -1;
}

ItemListModel::ItemListModel(GameWorld *world, QObject *parent) :
	QAbstractTableModel(parent), _world( world )
{
}

int ItemListModel::rowCount(const QModelIndex &parent) const
{
	return _world->objectTypes.size();
}

int ItemListModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

QVariant ItemListModel::data(const QModelIndex &index, int role) const
{
	if ( role == Qt::DisplayRole ) {
		qint16 id = getIDOf(index.row());
		if( id == -1 ) return QVariant::Invalid;
		if( index.column() == 0 ) {
			return id;
		}
		else if ( index.column() == 1 ) {
			return QString::fromStdString(
						world()->objectTypes[id]->modelName);
		}
	}
	return QVariant::Invalid;
}

QVariant ItemListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if(section == 0) {
			return "ID";
		}
		if(section == 1) {
			return "Model";
		}
	}
	return QVariant::Invalid;
}

QModelIndex ItemListModel::index(int row, int column, const QModelIndex &parent) const
{
	return hasIndex(row, column, parent) ? createIndex(row, column, getIDOf(row)) : QModelIndex();
}
