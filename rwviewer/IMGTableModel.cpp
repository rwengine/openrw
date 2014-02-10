#include "IMGTableModel.hpp"

QVariant IMGTableModel::data(const QModelIndex& index, int role) const
{
	if(role == Qt::DisplayRole) {
		if(index.row() < archive.getAssetCount()) {
			auto& f = archive.getAssetInfoByIndex(index.row());
			if(index.column() == 0) {
				return QString(f.name);
			}
			if(index.column() == 1) {
				return f.size;
			}
		}
	}
	return QVariant::Invalid;
}

QVariant IMGTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if(section == 0) {
			return "Name";
		}
		if(section == 1) {
			return "Size";
		}
	}
	return QVariant::Invalid;
}

int IMGTableModel::rowCount(const QModelIndex& parent) const
{
	return archive.getAssetCount();
}

int IMGTableModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

