#ifndef RWGAME_RWVIEWER_HPP
#define RWGAME_RWVIEWER_HPP

#include "GameBase.hpp"
#include "RWConfig.hpp"
#include "RWImGui.hpp"
#include "RWViewerWindows.hpp"
#include "game.hpp"

#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <fonts/GameTexts.hpp>
#include <render/DebugDraw.hpp>
#include <render/GameRenderer.hpp>

#include <SDL_events.h>

#include <chrono>
#include <optional>
#include <set>

class RWViewer final : public GameBase {
public:
    RWViewer(Logger& log, const std::optional<RWArgConfigLayer>& args);
    ~RWViewer() override;

    int run();

    GameWorld* getWorld() {
        return world_.get();
    }

    const GameData& getGameData() const {
        return data_;
    }

    GameRenderer& getRenderer() {
        return renderer_;
    }

private:
    void tick(float dt);
    void render(float alpha, float dt);

    void globalKeyEvent(const SDL_Event& event);

    bool updateInput();

    void drawMenu();
    void drawWindows();

    void drawModelWindow(ModelID);

    void showModel(ModelID model) {
        showModels_.insert(model);
    }

    void viewModel(ModelID model);

    GameData data_;
    GameRenderer renderer_;
    GameState state_;
    RWImGui imgui_;

    std::unique_ptr<GameWorld> world_;

    bool continue_ = true;

    bool showModelList_ = false;
    bool showTextViewer_ = false;
    std::optional<TextViewer> textViewer_;
    bool showImGuiDemo_ = false;
    bool showIplViewer_ = false;
    std::optional<IPLViewer> iplViewer_;
    std::set<ModelID> showModels_;

    enum class ViewMode {
        World,
        Model,
    };
    ViewMode viewMode_ = ViewMode::World;
    enum class MouseMode { Hovering, Dragging };
    MouseMode mouseMode_ = MouseMode::Hovering;

    GameObject* viewedObject_ = nullptr;
    glm::vec3 viewAngles_{};

    glm::vec3 viewPosition_{};
};

#endif
