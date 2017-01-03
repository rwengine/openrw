#pragma once
#ifndef _OBJECTVIEWER_HPP_
#define _OBJECTVIEWER_HPP_
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>

#include "ViewerInterface.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>

class ViewerWidget;
class Clump;

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

public:
    ObjectViewer(ViewerWidget* viewer = 0, QWidget* parent = 0,
                 Qt::WindowFlags f = 0);

    void setViewerWidget(ViewerWidget* widget);

protected:
    void worldChanged() override;

signals:

    void modelChanged(Clump* model);

    void showObjectModel(uint16_t object);

public slots:

    void showItem(qint16 item);

private slots:

    void showItem(QModelIndex model);

    void onCustomContextMenu(const QPoint&);
    void menuViewModel();
};

#endif
