#ifndef _OBJECTVIEWER_HPP_
#define _OBJECTVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>

#include "ViewerInterface.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QSortFilterProxyModel>

class ViewerWidget;
class Clump;

class ObjectSearchModel;

class ObjectViewer : public ViewerInterface {
    Q_OBJECT

    QTableView* objectList;

    QHBoxLayout* mainLayout;
    QGridLayout* infoLayout;
    ViewerWidget* previewWidget;
    QLabel* previewID;
    QLabel* previewModel;
    QLabel* previewClass;

    QMenu* objectMenu;
    QModelIndex contextMenuIndex;

    ObjectSearchModel* filterModel;

public:
    ObjectViewer(QWidget* parent = 0, Qt::WindowFlags f = 0);

protected:
    void worldChanged() override;

signals:
    void showObjectModel(uint16_t object);

public slots:
    void showItem(qint16 item);

private slots:

    void showItem(QModelIndex model);

    void onCustomContextMenu(const QPoint&);
    void menuViewModel();
};

#endif
