#ifndef LOADINGSTATE_HPP
#define LOADINGSTATE_HPP
#include "StateManager.hpp"

#include <functional>

class LoadingState : public State {
    std::function<void(void)> complete;

public:
    LoadingState(RWGame* game, std::function<void(void)> callback);

    virtual void enter();
    virtual void exit();

    virtual void tick(float dt);

    virtual void draw(GameRenderer* r);

    virtual bool shouldWorldUpdate();

    virtual void handleEvent(const SDL_Event& event);
};

#endif  // LOADINGSTATE_HPP
