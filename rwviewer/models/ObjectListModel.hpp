#ifndef _OBJECTLISTMODEL_HPP_
#define _OBJECTLISTMODEL_HPP_

#include <QAbstractItemModel>

#include <engine/GameData.hpp>

class ObjectListModel : public QAbstractTableModel {
    Q_OBJECT

    GameData* _gameData;

public:
    explicit ObjectListModel(GameData* gameDat, QObject* parent = 0);

    GameData* gameData() const {
        return _gameData;
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
};

#endif  // _OBJECTLISTMODEL_HPP_
