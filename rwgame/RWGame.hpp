#ifndef _RWGAME_HPP_
#define _RWGAME_HPP_

#include <core/Logger.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>
#include <script/ScriptMachine.hpp>
#include <chrono>
#include "game.hpp"

#include "GameConfig.hpp"
#include "GameWindow.hpp"

#include "SDL.h"

class PlayerController;

class RWGame
{
  Logger log;
  GameConfig config{"openrw.ini"};
  GameState* state = nullptr;
  GameData* data = nullptr;
  GameWorld* world = nullptr;
  // must be allocated after Logger setup.
  GameRenderer* renderer = nullptr;
  ScriptMachine* script = nullptr;
  GameWindow* window = nullptr;
  // Background worker
  WorkContext* work = nullptr;
  std::chrono::steady_clock clock;
  std::chrono::steady_clock::time_point last_clock_time;

  bool inFocus = true;
  ViewCamera lastCam, nextCam;
  bool showDebugStats = false;
  bool showDebugPaths = false;
  bool showDebugPhysics = false;
  int lastDraws;  /// Number of draws issued for the last frame.

  float accum = 0.f;
  float timescale = 1.f;

public:
  RWGame(int argc, char* argv[]);
  ~RWGame();

  int run();

  /**
   * Initalizes a new game
   */
  void newGame();

  GameState* getState() const { return state; }

  GameWorld* getWorld() const { return world; }

  GameData* getGameData() const { return data; }

  GameRenderer* getRenderer() const { return renderer; }

  GameWindow& getWindow() { return *window; }

  ScriptMachine* getScript() const { return script; }

  const GameConfig& getConfig() const { return config; }

  bool hitWorldRay(glm::vec3& hit, glm::vec3& normal, GameObject** object = nullptr)
  {
    auto vc = nextCam;
    glm::vec3 from(vc.position.x, vc.position.y, vc.position.z);
    glm::vec3 tmp = vc.rotation * glm::vec3(1000.f, 0.f, 0.f);

    return hitWorldRay(from, tmp, hit, normal, object);
  }

  bool hitWorldRay(const glm::vec3& start, const glm::vec3& direction, glm::vec3& hit,
                   glm::vec3& normal, GameObject** object = nullptr)
  {
    auto from = btVector3(start.x, start.y, start.z);
    auto to = btVector3(start.x + direction.x, start.y + direction.y, start.z + direction.z);
    btCollisionWorld::ClosestRayResultCallback ray(from, to);

    world->dynamicsWorld->rayTest(from, to, ray);
    if (ray.hasHit()) {
      hit = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(), ray.m_hitPointWorld.z());
      normal =
          glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(), ray.m_hitNormalWorld.z());
      if (object) {
        *object = static_cast<GameObject*>(ray.m_collisionObject->getUserPointer());
      }
      return true;
    }
    return false;
  }

  void startScript(const std::string& name);

  bool hasFocus() const { return inFocus; }

  void saveGame(const std::string& savename);
  void loadGame(const std::string& savename);

  /** shortcut for getWorld()->state.player->getCharacter() */
  PlayerController* getPlayer();

private:
  void tick(float dt);
  void render(float alpha, float dt);

  void renderDebugStats(float time, Renderer::ProfileInfo& worldRenderTime);
  void renderDebugPaths(float time);
  void renderProfile();

  void globalKeyEvent(const SDL_Event& event);
};

#endif
