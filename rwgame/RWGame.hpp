#ifndef RWGAME_RWGAME_HPP
#define RWGAME_RWGAME_HPP

#include <chrono>

// FIXME: should be in rwengine, deeply hidden
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <render/DebugDraw.hpp>
#include <render/GameRenderer.hpp>
#include <script/SCMFile.hpp>
#include <script/ScriptMachine.hpp>
#include <script/modules/GTA3Module.hpp>
#include "game.hpp"

#include "GameBase.hpp"

class PlayerController;

class RWGame final : public GameBase {
    GameData data;
    GameRenderer renderer;
    DebugDraw debug;
    GameState state;

    std::unique_ptr<GameWorld> world;

    GTA3Module opcodes;
    std::unique_ptr<ScriptMachine> vm;
    SCMFile script;

    bool inFocus = true;
    ViewCamera currentCam;

    enum class DebugViewMode {
        Disabled,
        General,
        Physics,
        Navigation,
        Objects
    };

    DebugViewMode debugview_ = DebugViewMode::Disabled;
    int lastDraws{0};  /// Number of draws issued for the last frame.

    std::string cheatInputWindow = std::string(32, ' ');

public:
    RWGame(Logger& log, int argc, char* argv[]);
    ~RWGame() override;

    int run();

    /**
     * Initalizes a new game
     */
    void newGame();

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

    bool hitWorldRay(glm::vec3& hit, glm::vec3& normal,
                     GameObject** object = nullptr) {
        auto vc = currentCam;
        glm::vec3 from(vc.position.x, vc.position.y, vc.position.z);
        glm::vec3 tmp = vc.rotation * glm::vec3(1000.f, 0.f, 0.f);

        return hitWorldRay(from, tmp, hit, normal, object);
    }

    bool hitWorldRay(const glm::vec3& start, const glm::vec3& direction,
                     glm::vec3& hit, glm::vec3& normal,
                     GameObject** object = nullptr) {
        auto from = btVector3(start.x, start.y, start.z);
        auto to = btVector3(start.x + direction.x, start.y + direction.y,
                            start.z + direction.z);
        btCollisionWorld::ClosestRayResultCallback ray(from, to);

        world->dynamicsWorld->rayTest(from, to, ray);
        if (ray.hasHit()) {
            hit = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
                            ray.m_hitPointWorld.z());
            normal =
                glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
                          ray.m_hitNormalWorld.z());
            if (object) {
                *object = static_cast<GameObject*>(
                    ray.m_collisionObject->getUserPointer());
            }
            return true;
        }
        return false;
    }

    void startScript(const std::string& name);

    bool hasFocus() const {
        return inFocus;
    }

    void saveGame(const std::string& savename);
    void loadGame(const std::string& savename);

private:
    void tick(float dt);
    void render(float alpha, float dt);

    void renderDebugStats(float time);
    void renderDebugPaths(float time);
    void renderDebugObjects(float time, ViewCamera& camera);

    void handleCheatInput(char symbol);

    void globalKeyEvent(const SDL_Event& event);

    bool updateInput();

    float tickWorld(const float deltaTime, float accumulatedTime);

    void renderDebugView(float time, ViewCamera &viewCam);

    void tickObjects(float dt) const;
};

#endif
