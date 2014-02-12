#include "DFFFramesTreeModel.hpp"
#include <render/Model.hpp>

DFFFramesTreeModel::DFFFramesTreeModel(Model* m, QObject* parent)
	: QAbstractItemModel(parent), model(m)
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
			for(int i = 0; i < cp->getParent()->getChildren().size(); ++i) {
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
	return QVariant();
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

