#include "ArchiveContentsWidget.hpp"
#include <QVBoxLayout>

ArchiveContentsWidget::ArchiveContentsWidget(QWidget* parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags), filter(nullptr), model(nullptr)
{
	setWindowTitle("Archive");
	QVBoxLayout* layout = new QVBoxLayout();
	QWidget* intermediate = new QWidget();

	searchbox = new QLineEdit();
	searchbox->setPlaceholderText("Search");

	table = new QListView();
	layout->addWidget(searchbox);
	layout->addWidget(table);
	intermediate->setLayout(layout);
	setWidget(intermediate);

	filter = new QSortFilterProxyModel;
	table->setModel(filter);
	connect(table->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(selectedIndexChanged(QModelIndex)));
	connect(searchbox, SIGNAL(textChanged(QString)), SLOT(setFilter(QString)));
}

void ArchiveContentsWidget::setArchive(const LoaderIMG& archive)
{
	auto m = new IMGArchiveModel(archive);
	filter->setSourceModel(m);
	if(model) {
		delete model;
	}
	model = m;
}

void ArchiveContentsWidget::selectedIndexChanged(const QModelIndex& current)
{
	auto mts = filter->mapToSource(current);
	if(mts.row() < model->getArchive().getAssetCount()) {
		auto& f = model->getArchive().getAssetInfoByIndex(mts.row());
		emit selectedFileChanged(f.name);
	}
}

void ArchiveContentsWidget::setFilter(const QString &f)
{
	filter->setFilterRegExp(QRegExp(f, Qt::CaseInsensitive));
}
