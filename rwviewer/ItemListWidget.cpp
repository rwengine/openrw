#include "ItemListWidget.hpp"
#include <QVBoxLayout>
#include <QHeaderView>

ItemListWidget::ItemListWidget(QWidget* parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags), filter(nullptr), model(nullptr)
{
	setWindowTitle("Items");
	QVBoxLayout* layout = new QVBoxLayout();
	QWidget* intermediate = new QWidget();

	searchbox = new QLineEdit();
	searchbox->setPlaceholderText("Search");

	table = new QTableView();
	layout->addWidget(searchbox);
	layout->addWidget(table);
	intermediate->setLayout(layout);
	setWidget(intermediate);

	filter = new QSortFilterProxyModel;
	table->setModel(filter);
	filter->setFilterKeyColumn(-1); // Search all columns
	connect(table->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(selectedIndexChanged(QModelIndex)));
	connect(searchbox, SIGNAL(textChanged(QString)), SLOT(setFilter(QString)));
}

void ItemListWidget::worldLoaded(GameWorld *world)
{
	if ( model ) delete model;
	model = new ItemListModel( world, this );
	filter->setSourceModel( model );
}

void ItemListWidget::selectedIndexChanged(const QModelIndex& current)
{
	auto mts = filter->mapToSource(current);
	if( mts.isValid() ) {
		emit selectedItemChanged( current.internalId() );
	}
}

void ItemListWidget::setFilter(const QString &f)
{
	filter->setFilterRegExp(QRegExp(f, Qt::CaseInsensitive));
}
