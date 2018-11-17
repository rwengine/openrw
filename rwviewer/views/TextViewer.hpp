#ifndef _TEXTVIEWER_HPP_
#define _TEXTVIEWER_HPP_

#include "ViewerInterface.hpp"

#include <fonts/GameTexts.hpp>
#include <rw/filesystem.hpp>

#include <QTableView>

#include <vector>

class TextModel;
class ViewerWidget;

class QLayout;
class QItemSelection;
class QLineEdit;
class QTextEdit;
class QModelIndex;
class QString;
class QWidget;

class TextTableView : public QTableView {
    Q_OBJECT
protected slots:
    virtual void selectionChanged(const QItemSelection& selected,
                                  const QItemSelection& deselected) override;
signals:
    void gameStringChanged(const GameString& string);
};

class TextViewer : public ViewerInterface {
    Q_OBJECT

    TextModel* textModel;
    TextTableView* textTable;
    ViewerWidget* viewerWidget;

    QLineEdit* hexLineEdit;
    QTextEdit* textEdit;

    void worldChanged() override;

    GameString currentGameString;
    font_t currentFont;
    int currentFontSize;

    void updateRender();

    void setGameString(const GameString& gameString);
    std::vector<std::string> getFontTextureNames();

public:
    TextViewer(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

signals:
    void fontChanged(font_t font);
private slots:
    void onGameStringChange(const GameString& gameString);
    void onStringChange();
    void onFontChange(size_t font);
    void onFontSizeChange(int font);
};

#endif
