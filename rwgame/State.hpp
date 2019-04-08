#ifndef RWGAME_STATE_HPP
#define RWGAME_STATE_HPP

#include "MenuSystem.hpp"

#include <SDL_events.h>

#include <memory>
#include <optional>
#include <utility>

class RWGame;
class GameWorld;
class LegacyGameRenderer;
class GameWindow;
class Menu;
class StateManager;
class ViewCamera;

class State {
public:
    RWGame* game;

    State(RWGame* game);

    virtual void enter() = 0;
    virtual void exit() = 0;

    virtual void tick(float dt) = 0;

    virtual void draw(LegacyGameRenderer& r);

    virtual ~State() = default;

    template<typename T>
    void setNextMenu(T&& menu) {
        nextMenu = std::forward<T>(menu);
    }

    std::optional<Menu>& getCurrentMenu();

    virtual void handleEvent(const SDL_Event& e);

    virtual const ViewCamera& getCamera(float alpha);

    /**
     * Returns false if the game world should not should
     * not update while this state is active
     */
    virtual bool shouldWorldUpdate();

    GameWorld* getWorld() const;
    GameWindow& getWindow();

    bool hasExited() const;

private:
    bool hasExited_ = false;

    void refreshMenu();

protected:
    std::optional<Menu> menu;
    std::optional<Menu> nextMenu;

    void done();
};

#endif
