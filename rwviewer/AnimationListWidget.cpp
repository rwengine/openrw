#include "AnimationListWidget.hpp"
#include <QVBoxLayout>

AnimationListWidget::AnimationListWidget(QWidget* parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags), filter(nullptr), model(nullptr)
{
	setWindowTitle("Animations");
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

void AnimationListWidget::setAnimations(const AnimationList& archive)
{
	auto m = new AnimationListModel(archive);
	filter->setSourceModel(m);
	if(model) {
		delete model;
	}
	model = m;
}

void AnimationListWidget::selectedIndexChanged(const QModelIndex& current)
{
	auto mts = filter->mapToSource(current);
	if(mts.row() < model->getAnimations().size()) {
		auto& f = model->getAnimations().at(mts.row());
		emit selectedAnimationChanged(f.second);
	}
}

void AnimationListWidget::setFilter(const QString &f)
{
	filter->setFilterRegExp(QRegExp(f, Qt::CaseInsensitive));
}
