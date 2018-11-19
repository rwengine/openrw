#ifndef _MODELFRAMESWIDGET_HPP_
#define _MODELFRAMESWIDGET_HPP_
#include <QDockWidget>
#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>
#include <rw/types.hpp>
#include "models/DFFFramesTreeModel.hpp"

class ModelFrame;

class ModelFramesWidget : public QWidget {
    Q_OBJECT

    ClumpPtr gmodel;
    DFFFramesTreeModel* framemodel;
    QTreeView* tree;
    QVBoxLayout* _layout;
    QLabel* _frameLabel;

private slots:

    void updateInfoBox(const ClumpPtr& model, ModelFrame* f);

    void selectedModelChanged(const QModelIndex&, const QModelIndex&);

public:
    ModelFramesWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:

    void setModel(const ClumpPtr& model);

signals:

    void selectedFrameChanged(ModelFrame* frame);
};

#endif
