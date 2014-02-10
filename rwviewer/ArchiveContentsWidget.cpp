#include "ArchiveContentsWidget.hpp"

ArchiveContentsWidget::ArchiveContentsWidget(QWidget* parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags), model(nullptr)
{
	setWindowTitle("Archive");
	table = new QTableView();
	table->setShowGrid(false);
	setWidget(table);
}

void ArchiveContentsWidget::setArchive(const LoaderIMG& archive)
{
	auto m = table->model();
	model = new IMGTableModel(archive);
	table->setModel(model);
	delete m;
}
