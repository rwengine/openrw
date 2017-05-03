#ifndef PAUSESTATE_HPP
#define PAUSESTATE_HPP

#include "StateManager.hpp"

class PauseState : public State {
public:
    PauseState(RWGame* game);

    virtual void enter() override;
    virtual void exit() override;

    virtual void tick(float dt) override;

    virtual void draw(GameRenderer* r) override;

    virtual void handleEvent(const SDL_Event& event) override;
};

#endif  // PAUSESTATE_HPP
