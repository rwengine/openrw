#include "DFFFramesTreeModel.hpp"
#include <render/Model.hpp>
#include <data/Skeleton.hpp>

DFFFramesTreeModel::DFFFramesTreeModel(Model *m, Skeleton* skel, QObject* parent)
	: QAbstractItemModel(parent), model(m), skeleton(skel)
{

}

int DFFFramesTreeModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

int DFFFramesTreeModel::rowCount(const QModelIndex& parent) const
{
	ModelFrame* f = static_cast<ModelFrame*>(parent.internalPointer());
	if(f) {
		return f->getChildren().size();
	}
	
	if(parent.row() == -1) {
		return 1;
	}
	
	return 0;
}

QModelIndex DFFFramesTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if(parent.row() == -1 && parent.column() == -1) {
		return createIndex(row, column, model->frames[model->rootFrameIdx]);
	}
	ModelFrame* f = static_cast<ModelFrame*>(parent.internalPointer());
	ModelFrame* p = f->getChildren()[row];
	return createIndex(row, column, p);
}

QModelIndex DFFFramesTreeModel::parent(const QModelIndex& child) const
{
	ModelFrame* c = static_cast<ModelFrame*>(child.internalPointer());
	if(c->getParent()) {
		auto cp = c->getParent();
		if(cp->getParent()) {
			for(size_t i = 0; i < cp->getParent()->getChildren().size(); ++i) {
				if(cp->getParent()->getChildren()[i] == c->getParent()) {
					return createIndex(i, 0, c->getParent());
				}
			}
		}
		else {
			return createIndex(0,0, cp);
		}
	}
	return QModelIndex();
}

QVariant DFFFramesTreeModel::data(const QModelIndex& index, int role) const
{
	ModelFrame* f = static_cast<ModelFrame*>(index.internalPointer());
	if(role == Qt::DisplayRole) {
		if(index.column() == 0) {
			return QString(f->getName().c_str());
		}
	}
	else if( role == Qt::CheckStateRole )
	{
		if( index.column() == 0 )
		{
			if( skeleton )
			{
				return skeleton->getData(f->getIndex()).enabled ?
					Qt::Checked : Qt::Unchecked;
			}
		}
	}
	return QVariant();
}

bool DFFFramesTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(! index.isValid() )
	{
		return false;
	}

	ModelFrame* f = static_cast<ModelFrame*>(index.internalPointer());

	if( role == Qt::CheckStateRole )
	{
		if( index.column() == 0 && skeleton )
		{
			if( (Qt::CheckState)value.toInt() == Qt::Checked )
			{
				skeleton->setEnabled(f, true);
			}
			else
			{
				skeleton->setEnabled(f, false);
			}
			return true;
		}
	}

	return false;
}

Qt::ItemFlags DFFFramesTreeModel::flags(const QModelIndex& index) const
{
	if(! index.isValid() )
	{
		return 0;
	}

	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if( index.column() == 0 && skeleton )
	{
		flags |= Qt::ItemIsUserCheckable;
	}

	return flags;
}

QVariant DFFFramesTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal) {
		if(role == Qt::DisplayRole) {
			return "Name";
		}
	}
	return QVariant();
}

