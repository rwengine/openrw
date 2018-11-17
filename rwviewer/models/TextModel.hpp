#ifndef _TEXTMODEL_HPP_
#define _TEXTMODEL_HPP_

#include <map>
#include <string>
#include <vector>

#include <QAbstractTableModel>

#include <fonts/GameTexts.hpp>

struct TextMapType {
    std::vector<std::string> languages;
    std::vector<std::string> keys;
    std::map<std::string, std::map<std::string, GameString>> map_lang_key_tran;
};

class TextModel : public QAbstractTableModel {
    Q_OBJECT
public:
    TextModel(QObject* parent = nullptr);
    void setMapData(const TextMapType& textMap);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    const GameString& lookupIndex(const QModelIndex& index) const;
public slots:
    void fontChanged(font_t font);

private:
    font_t m_font = FONT_PAGER;
    TextMapType m_textMap;
};

#endif
