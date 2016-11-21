#pragma once
#ifndef _ANIMATIONLISTMODEL_HPP_
#define _ANIMATIONLISTMODEL_HPP_
#include <QAbstractItemModel>
#include <loaders/LoaderIFP.hpp>

typedef std::vector<std::pair<std::string, Animation*>> AnimationList;

class AnimationListModel : public QAbstractListModel {
    Q_OBJECT

    AnimationList animations;

public:
    AnimationListModel(const AnimationList& anims, QObject* parent = 0)
        : QAbstractListModel(parent), animations(anims) {
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    const AnimationList& getAnimations() const {
        return animations;
    }
};

#endif
