#ifndef _IMGARCHIVEMODEL_HPP_
#define _IMGARCHIVEMODEL_HPP_
#include <QAbstractItemModel>
#include <loaders/LoaderIMG.hpp>

class IMGArchiveModel : public QAbstractListModel {
    Q_OBJECT

    LoaderIMG archive;

public:
    IMGArchiveModel(const LoaderIMG& archive, QObject* parent = 0)
        : QAbstractListModel(parent), archive(archive) {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    const LoaderIMG& getArchive() const {
        return archive;
    }
};

#endif
