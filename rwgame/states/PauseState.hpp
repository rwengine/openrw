#ifndef PAUSESTATE_HPP
#define PAUSESTATE_HPP

#include "State.hpp"

class PauseState final : public State {
public:
    PauseState(RWGame* game);

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(GameRenderer &r) override;

    void handleEvent(const SDL_Event& event) override;
};

#endif  // PAUSESTATE_HPP
