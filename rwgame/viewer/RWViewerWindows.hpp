#ifndef RWGAME_RWVIEWERWINDOWS_HPP
#define RWGAME_RWVIEWERWINDOWS_HPP

#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <fonts/GameTexts.hpp>
#include <loaders/LoaderIPL.hpp>
#include <objects/GameObject.hpp>
#include <render/GameRenderer.hpp>

#include <core/Logger.hpp>

#include <string>
#include <vector>

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

class IPLViewer {
public:
    IPLViewer(Logger&, GameData& data);

    void draw(GameRenderer& r);

    GameWorld* world() const {
        return world_.get();
    }

private:
    struct IPLFileData {
        bool enabled;
        std::string name;
        std::string path;
        LoaderIPL loader;
        std::vector<GameObject*> objects;
    };

    void showIPL(IPLFileData&);
    void hideIPL(IPLFileData&);

    GameData& data_;
    GameState state_;
    std::unique_ptr<GameWorld> world_;
    std::vector<IPLFileData> ipls_;
};

#endif  // RWGAME_RWVIEWERWINDOWS_HPP
