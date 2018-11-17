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

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex& parent) const override;
};

#endif  // ITEMLISTMODEL_HPP
