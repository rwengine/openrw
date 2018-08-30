#pragma once
#ifndef _ANIMATIONLISTMODEL_HPP_
#define _ANIMATIONLISTMODEL_HPP_
#include <QAbstractItemModel>
#include <loaders/LoaderIFP.hpp>

typedef std::vector<std::pair<std::string, AnimationPtr>> AnimationList;

class AnimationListModel : public QAbstractListModel {
    Q_OBJECT

    AnimationList animations;

public:
    AnimationListModel(const AnimationList& anims, QObject* parent = 0)
        : QAbstractListModel(parent), animations(anims) {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    const AnimationList& getAnimations() const {
        return animations;
    }
};

#endif
