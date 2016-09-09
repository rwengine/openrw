#pragma once
#ifndef _ITEMLISTWIDGET_HPP_
#define _ITEMLISTWIDGET_HPP_
#include <QDockWidget>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>
#include "ItemListModel.hpp"

class ItemListWidget : public QDockWidget {
    Q_OBJECT

    QSortFilterProxyModel* filter;
    ItemListModel* model;
    QTableView* table;
    QLineEdit* searchbox;

public:
    ItemListWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

signals:
    void selectedItemChanged(const qint16 id);

public slots:

    void worldLoaded(GameWorld* world);

    void selectedIndexChanged(const QModelIndex& current);

    void setFilter(const QString& f);
};

#endif
