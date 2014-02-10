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
}
