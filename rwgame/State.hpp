#ifndef RWGAME_STATE_HPP
#define RWGAME_STATE_HPP

#include <optional>

#include <render/ViewCamera.hpp>
#include "GameWindow.hpp"
#include "MenuSystem.hpp"

#include <SDL.h>
#include <SDL_events.h>

class RWGame;
class GameWorld;
class StateManager;

class State {
public:
    RWGame* game;

    State(RWGame* game);

    virtual void enter() = 0;
    virtual void exit() = 0;

    virtual void tick(float dt) = 0;

    virtual void draw(GameRenderer* r);

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
