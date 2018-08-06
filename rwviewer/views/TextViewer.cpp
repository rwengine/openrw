#include "TextViewer.hpp"

#include <render/TextRenderer.hpp>
#include <rw/filesystem.hpp>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>

#include <boost/filesystem.hpp>
#include <fonts/GameTexts.hpp>
#include <loaders/LoaderGXT.hpp>
#include <models/TextModel.hpp>
#include <platform/FileHandle.hpp>
#include <render/GameRenderer.hpp>

#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QItemSelection>
#include <QLineEdit>
#include <QModelIndex>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSpinBox>
#include <QSplitter>
#include <QTableView>
#include <QTextEdit>

void TextTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &) {
    if (!selected.size())
        return;
    auto index = selected.indexes()[0];
    if (!index.isValid())
        return;

    auto *textModel = dynamic_cast<TextModel *>(this->model());
    if (!textModel) {
        return;
    }
    try {
        auto gameString = textModel->lookupIndex(index);
        emit gameStringChanged(gameString);
    } catch (std::out_of_range &) {
        return;
    }
}

TextViewer::TextViewer(QWidget* parent, Qt::WindowFlags f)
    : ViewerInterface(parent, f) {

    auto dataLayout = new QVBoxLayout;

    auto splitter = new QSplitter;
    splitter->setChildrenCollapsible(false);
    splitter->setOrientation(Qt::Horizontal);

    viewerWidget = createViewer();

    textModel = new TextModel;
    textTable = new TextTableView;
    textTable->setModel(textModel);
    textTable->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(textTable, &TextTableView::gameStringChanged, this, &TextViewer::onGameStringChange);
    dataLayout->addWidget(textTable);

    auto propLayout = new QHBoxLayout;

    connect(this, &TextViewer::fontChanged, this, &TextViewer::onFontChange);
    auto groupBox = new QGroupBox;
    auto *groupBoxLayout = new QHBoxLayout;
    auto *radioFont1 = new QRadioButton(tr("Pager"));
    connect(radioFont1, &QRadioButton::clicked, [this]() {emit fontChanged(FONT_PAGER);});
    groupBoxLayout->addWidget(radioFont1);
    auto *radioFont2 = new QRadioButton(tr("Pricedown"));
    connect(radioFont2, &QRadioButton::clicked, [this]() {emit fontChanged(FONT_PRICEDOWN);});
    groupBoxLayout->addWidget(radioFont2);
    auto *radioFont3 = new QRadioButton(tr("Arial"));
    connect(radioFont3, &QRadioButton::clicked, [this]() {emit fontChanged(FONT_ARIAL);});
    groupBoxLayout->addWidget(radioFont3);
    groupBox->setLayout(groupBoxLayout);
    groupBox->setProperty("border", "2px solid gray");
    propLayout->addWidget(groupBox);

    radioFont1->click();

    auto textSizeLayout = new QFormLayout;
    auto textSizeSpinBox = new QSpinBox;
    textSizeSpinBox->setMinimum(1);
    textSizeSpinBox->setMaximum(50);
    connect(textSizeSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &TextViewer::onFontSizeChange);
    textSizeSpinBox->setValue(20);
    textSizeLayout->addRow(tr("Font size"), textSizeSpinBox);
    propLayout->addLayout(textSizeLayout);

    propLayout->addStretch();
    dataLayout->addLayout(propLayout);

    hexLineEdit = new QLineEdit;
    hexLineEdit->setReadOnly(true);
    hexLineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9A-F]*")));
    dataLayout->addWidget(hexLineEdit);

    textEdit = new QTextEdit;
    dataLayout->addWidget(textEdit);

    auto dataWidget = new QWidget;
    dataWidget->setLayout(dataLayout);
    splitter->addWidget(dataWidget);

    viewerWidget->setMode(ViewerWidget::Mode::Text);
    splitter->addWidget(QWidget::createWindowContainer(viewerWidget));

    auto mainLayout = new QHBoxLayout;
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    connect(textSizeSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &TextViewer::onFontSizeChange);
    connect(textEdit, &QTextEdit::textChanged, this, &TextViewer::onStringChange);
}

void TextViewer::onFontChange(font_t font) {
    currentFont = font;
    emit textModel->fontChanged(font);
    updateRender();
}

void TextViewer::onFontSizeChange(int size) {
    currentFontSize = size;
    updateRender();
}

void TextViewer::onStringChange() {
    auto string = textEdit->toPlainText();
    auto newGameString = GameStringUtil::fromString(string.toStdString(), currentFont);
    onGameStringChange(newGameString);
}

void TextViewer::onGameStringChange(const GameString &gameString) {
    if (!currentGameString.compare(gameString)) {
        return;
    }
    currentGameString = gameString;

    std::ostringstream oss;
    oss << std::hex;
    for (auto c : gameString) {
        oss << std::setw(sizeof(gameString[0])) << std::setfill('0')
                << int(c) << " ";
    }
    auto newHexText = QString::fromStdString(oss.str());
    if (hexLineEdit->text().compare(newHexText)) {
        hexLineEdit->setText(newHexText);
    }
    auto newText = QString::fromStdString(GameStringUtil::toString(gameString, currentFont));
    if (textEdit->toPlainText().compare(newText)) {
        textEdit->setText(newText);
    }

    updateRender();
}

void TextViewer::updateRender() {
    viewerWidget->clearText();
    const int ROW_STRIDE = currentFontSize * 1.2;
    const int COL_STRIDE = currentFontSize * 1.2;
    {
        TextRenderer::TextInfo ti;
        ti.font = currentFont;
        ti.size = currentFontSize;
        ti.baseColour = glm::u8vec3(255);
        ti.backgroundColour = glm::u8vec4(0, 0, 0, 0);
        ti.align = TextRenderer::TextInfo::TextAlignment::Left;
        ti.wrapX = 0;

        ti.text = currentGameString;
        ti.screenPosition = glm::vec2(10, 10);
        viewerWidget->showText(ti);
    }

    {
        TextRenderer::TextInfo ti;
        ti.font = currentFont;
        ti.size = currentFontSize;
        ti.baseColour = glm::u8vec3(255);
        ti.backgroundColour = glm::u8vec4(0, 0, 0, 0);
        ti.align = TextRenderer::TextInfo::TextAlignment::Left;
        ti.wrapX = 0;

        for(GameStringChar c=0x20; c<0xb2; ++c) {
            unsigned column = c % 0x10;
            unsigned row = (c / 0x10) - 2 + 3; /* +3 to offset first line*/
            ti.text = c;
            ti.screenPosition = glm::vec2(10 + (column * COL_STRIDE), 10 + (row * ROW_STRIDE));
            viewerWidget->showText(ti);
        }
    }
}

void TextViewer::worldChanged() {
    auto textNames = getFontTextureNames();
    TextMapType textMap;
    LoaderGXT loader;
    std::set<std::string> keys;
    for(const auto &textName : textNames) {
        GameTexts texts;
        auto handle = world()->data->index.openFile(textName);
        loader.load(texts, handle);
        const auto &language = textName;
        textMap.languages.push_back(language);
        const auto &stringTable = texts.getStringTable();
        for (const auto &tableItem : stringTable) {
            keys.insert(tableItem.first);
            textMap.map_lang_key_tran[language][tableItem.first] = tableItem.second;
        }
    }
    textMap.keys.resize(keys.size());
    std::move(keys.begin(), keys.end(), textMap.keys.begin());

    textModel->setData(textMap);
}

std::vector<std::string> TextViewer::getFontTextureNames() {
    const auto &gameDataPath = rwfs::path(world()->data->getDataPath());
    rwfs::path textPath;
    for (const rwfs::path &p : rwfs::directory_iterator(gameDataPath)) {
        if (!rwfs::is_directory(p)) {
            continue;
        }
        std::string filename = p.filename().string();
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
        if (!filename.compare("text")) {
            textPath = p;
            break;
        }
    }
    if (!textPath.string().length()) {
        throw std::runtime_error("text directory not found in gamedata path");
    }
    std::vector<std::string> names;
    for (const rwfs::path &p : rwfs::directory_iterator(textPath)) {
//        auto langName = p.lexically_relative(gameDataPath).string();
        auto langName = p.filename().string();
        std::transform(langName.begin(), langName.end(), langName.begin(), ::tolower);
        names.push_back(langName);
    }
    return names;
}
