#ifndef _GAME_STATE_HPP_
#define _GAME_STATE_HPP_
#include <functional>
#include <queue>
#include <render/ViewCamera.hpp>
#include "MenuSystem.hpp"
#include <glm/gtc/quaternion.hpp>
#include "SDL.h"
#include "SDL_events.h"
#include "GameWindow.hpp"

class RWGame;
class GameWorld;

struct State {
  // Helper for global menu behaviour
  Menu* currentMenu;
  Menu* nextMenu;

  RWGame* game;

  State(RWGame* game) : currentMenu(nullptr), nextMenu(nullptr), game(game) {}

  virtual void enter() = 0;
  virtual void exit() = 0;

  virtual void tick(float dt) = 0;

  virtual void draw(GameRenderer* r)
  {
    if (getCurrentMenu()) {
      getCurrentMenu()->draw(r);
    }
  }

  virtual ~State()
  {
    if (getCurrentMenu()) {
      delete getCurrentMenu();
    }
  }

  void enterMenu(Menu* menu) { nextMenu = menu; }

  Menu* getCurrentMenu()
  {
    if (nextMenu) {
      if (currentMenu) {
        delete currentMenu;
      }
      currentMenu = nextMenu;
      nextMenu = nullptr;
    }
    return currentMenu;
  }

  virtual void handleEvent(const SDL_Event& e);

  virtual const ViewCamera& getCamera();

  /**
   * Returns false if the game world should not should
   * not update while this state is active
   */
  virtual bool shouldWorldUpdate();

  GameWorld* getWorld();
  GameWindow& getWindow();
};

struct StateManager {
  static StateManager& get()
  {
    static StateManager m;
    return m;
  }

  std::deque<State*> states;

  void clear() { states.clear(); }

  void enter(State* state)
  {
    states.push_back(state);
    state->enter();
  }

  void exec(State* state)
  {
    exit();
    enter(state);
  }

  void tick(float dt) { states.back()->tick(dt); }

  void draw(GameRenderer* r) { states.back()->draw(r); }

  void exit()
  {
    // TODO: Resole states being leaked.
    states.back()->exit();
    states.pop_back();
    if (states.size() > 0) {
      states.back()->enter();
    }
  }
};

#endif
