#ifndef RWGAME_RWVIEWERWINDOWS_HPP
#define RWGAME_RWVIEWERWINDOWS_HPP

#include <engine/GameData.hpp>
#include <fonts/GameTexts.hpp>
#include <render/GameRenderer.hpp>

#include <string>

class TextViewer {
public:
    explicit TextViewer(GameData& data);

    void cacheStrings();
    void draw(GameRenderer& r);

private:
    GameData& data_;
    std::vector<std::string> languages_;
    int currentFont_ = 0;
    float currentFontSize_ = 20.f;
    using StringPair = std::pair<GameString, std::string>;
    std::map<GameStringKey, std::map<std::string, StringPair>> texts_;
    char previewStr_[512] = {};
    GameString previewText_;
    bool drawPreview_ = true;
};

#endif  // RWGAME_RWVIEWERWINDOWS_HPP
