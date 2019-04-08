#ifndef LOADINGSTATE_HPP
#define LOADINGSTATE_HPP

#include "State.hpp"

#include <functional>

class LoadingState final : public State {
    std::function<void(void)> complete;

public:
    LoadingState(RWGame* game, const std::function<void(void)>& callback);

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(LegacyGameRenderer& r) override;

    bool shouldWorldUpdate() override;

    void handleEvent(const SDL_Event& event) override;
};

#endif  // LOADINGSTATE_HPP
