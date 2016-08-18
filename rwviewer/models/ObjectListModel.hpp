#ifndef _OBJECTLISTMODEL_HPP_
#define _OBJECTLISTMODEL_HPP_

#include <QAbstractItemModel>

#include <engine/GameData.hpp>

class ObjectListModel : public QAbstractTableModel
{
  Q_OBJECT

  GameData* _gameData;

public:
  explicit ObjectListModel(GameData* gameDat, QObject* parent = 0);

  GameData* gameData() const { return _gameData; }

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;

  QModelIndex index(int row, int column, const QModelIndex& parent) const;
};

#endif  // _OBJECTLISTMODEL_HPP_
