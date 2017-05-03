#ifndef LOADINGSTATE_HPP
#define LOADINGSTATE_HPP
#include "StateManager.hpp"

#include <functional>

class LoadingState : public State {
    std::function<void(void)> complete;

public:
    LoadingState(RWGame* game, std::function<void(void)> callback);

    virtual void enter() override;
    virtual void exit() override;

    virtual void tick(float dt) override;

    virtual void draw(GameRenderer* r) override;

    virtual bool shouldWorldUpdate() override;

    virtual void handleEvent(const SDL_Event& event) override;
};

#endif  // LOADINGSTATE_HPP
