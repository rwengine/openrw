#ifndef RWGAME_RWGAME_HPP
#define RWGAME_RWGAME_HPP

#include "GameBase.hpp"
#include "HUDDrawer.hpp"
#include "RWConfig.hpp"
#include "RWImGui.hpp"
#include "StateManager.hpp"
#include "game.hpp"

#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <render/DebugDraw.hpp>
#include <render/GameRenderer.hpp>
#include <script/SCMFile.hpp>
#include <script/ScriptMachine.hpp>
#include <script/modules/GTA3Module.hpp>

#include <SDL_events.h>

#include <chrono>

class RWGame final : public GameBase {
public:
    enum class DebugViewMode {
        Disabled,
        General,
        Physics,
        Navigation,
        Objects
    };

private:
    GameData data;
    GameRenderer renderer;
    RWImGui imgui;
    DebugDraw debug;
    GameState state;
    HUDDrawer hudDrawer{};

    std::unique_ptr<GameWorld> world;

    GTA3Module opcodes;
    std::unique_ptr<ScriptMachine> vm;
    SCMFile script;

    StateManager stateManager;

    bool inFocus = true;
    ViewCamera currentCam;

    DebugViewMode debugview_ = DebugViewMode::Disabled;
    int lastDraws{0};  /// Number of draws issued for the last frame.

    std::string cheatInputWindow = std::string(32, ' ');

public:
    RWGame(Logger& log, const std::optional<RWArgConfigLayer> &args);
    ~RWGame() override;

    int run();

    /**
     * Initalizes a new game
     */
    void newGame();

    StateManager& getStateManager() {
        return stateManager;
    }

    GameState* getState() {
        return &state;
    }

    GameWorld* getWorld() {
        return world.get();
    }

    const GameData& getGameData() const {
        return data;
    }

    GameRenderer& getRenderer() {
        return renderer;
    }

    ScriptMachine* getScriptVM() const {
        return vm.get();
    }

    HUDDrawer& getHUDDrawer() {
        return hudDrawer;
    }

    DebugViewMode getDebugViewMode() const {
        return debugview_;
    }

    bool hitWorldRay(glm::vec3& hit, glm::vec3& normal,
                     GameObject** object = nullptr);

    bool hitWorldRay(const glm::vec3& start, const glm::vec3& direction,
                     glm::vec3& hit, glm::vec3& normal,
                     GameObject** object = nullptr);

    void startScript(const std::string& name);

    bool hasFocus() const {
        return inFocus;
    }

    void saveGame(const std::string& savename);
    void loadGame(const std::string& savename);

private:
    void tick(float dt);
    void render(float alpha, float dt);

    void renderDebugPaths();

    void handleCheatInput(char symbol);

    void globalKeyEvent(const SDL_Event& event);

    bool updateInput();

    float tickWorld(const float deltaTime, float accumulatedTime);

    void renderDebugView();

    void tickObjects(float dt) const;

    void interpolateWeather(uint8_t prevGameHour);
};

#endif

