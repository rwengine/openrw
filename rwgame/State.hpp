#ifndef RWGAME_STATE_HPP
#define RWGAME_STATE_HPP
#include <render/ViewCamera.hpp>
#include "GameWindow.hpp"
#include "MenuSystem.hpp"
#include "SDL.h"
#include "SDL_events.h"

class RWGame;
class GameWorld;
class StateManager;

class State {
public:
    // Helper for global menu behaviour
    Menu* currentMenu;
    Menu* nextMenu;

    RWGame* game;

    State(RWGame* game) : currentMenu(nullptr), nextMenu(nullptr), game(game) {
    }

    virtual void enter() = 0;
    virtual void exit() = 0;

    virtual void tick(float dt) = 0;

    virtual void draw(GameRenderer* r) {
        if (getCurrentMenu()) {
            getCurrentMenu()->draw(r);
        }
    }

    virtual ~State() {
        if (getCurrentMenu()) {
            delete getCurrentMenu();
        }
    }

    void enterMenu(Menu* menu) {
        nextMenu = menu;
    }

    Menu* getCurrentMenu() {
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

    bool hasExited() const {
        return hasexited_;
    }

private:
    bool hasexited_ = false;
protected:
    void done() { hasexited_ = true; }
};

#endif
