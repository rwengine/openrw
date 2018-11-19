#include "TextModel.hpp"
#include <iostream>

#include <QBrush>

TextModel::TextModel(QObject *parent)
     : QAbstractTableModel(parent) {

}

void TextModel::setMapData(const TextMapType &textMap) {
    beginResetModel();
    m_textMap = textMap;
    endResetModel();
}

int TextModel::rowCount(const QModelIndex & /*parent*/) const {
    return static_cast<int>(m_textMap.keys.size());
}

int TextModel::columnCount(const QModelIndex & /*parent*/) const {
    return static_cast<int>(m_textMap.languages.size());
}

const GameString &TextModel::lookupIndex(const QModelIndex &index) const {
    const auto &language = m_textMap.languages.at(index.column());
    const auto &key = m_textMap.keys.at(index.row());
    return m_textMap.map_lang_key_tran.at(language).at(key);
}

QVariant TextModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        try {
            const auto &gameText = this->lookupIndex(index);
            auto gameString = GameStringUtil::toString(gameText, m_font);
            return QString::fromStdString(gameString);
        } catch (const std::out_of_range &) {
            return QVariant::Invalid;
        } catch (...) {
            throw;
        }
    case Qt::BackgroundRole:
        try {
            this->lookupIndex(index);
            return QVariant::Invalid;
        } catch (const std::out_of_range &) {
            return QBrush(Qt::red);
        }
        break;
    default:
        return QVariant::Invalid;
    }
}

QVariant TextModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        switch (orientation) {
        case Qt::Horizontal:
            return QString::fromStdString(m_textMap.languages[section]);
        case Qt::Vertical:
            return QString::fromStdString(m_textMap.keys[section]);
        default:
            return QVariant::Invalid;
        }
    default:
    return QVariant::Invalid;
    }
}

void TextModel::fontChanged(font_t font) {
    beginResetModel();
    m_font = font;
    endResetModel();
}
