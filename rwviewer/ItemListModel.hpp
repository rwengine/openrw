#ifndef ITEMLISTMODEL_HPP
#define ITEMLISTMODEL_HPP

#include <QAbstractItemModel>

#include <engine/GameWorld.hpp>

class ItemListModel : public QAbstractTableModel {
    Q_OBJECT

    GameWorld* _world;

public:
    explicit ItemListModel(GameWorld* _world, QObject* parent = 0);

    GameWorld* world() const {
        return _world;
    }

    qint16 getIDOf(unsigned int row) const;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
};

#endif  // ITEMLISTMODEL_HPP
