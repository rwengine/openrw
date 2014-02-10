#include "ArchiveContentsWidget.hpp"

ArchiveContentsWidget::ArchiveContentsWidget(QWidget* parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags), model(nullptr)
{
	setWindowTitle("Archive");
	table = new QListView();
	setWidget(table);
}

void ArchiveContentsWidget::setArchive(const LoaderIMG& archive)
{
	auto m = table->model();
	model = new IMGArchiveModel(archive);
	table->setModel(model);
	delete m;
	connect(table->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(selectedIndexChanged(QModelIndex)));
}

void ArchiveContentsWidget::selectedIndexChanged(const QModelIndex& current)
{
	if(current.row() < model->getArchive().getAssetCount()) {
		auto& f = model->getArchive().getAssetInfoByIndex(current.row());
		emit selectedFileChanged(f.name);
	}
}
