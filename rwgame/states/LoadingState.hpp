#ifndef LOADINGSTATE_HPP
#define LOADINGSTATE_HPP
#include "StateManager.hpp"

#include <functional>

class LoadingState final : public State {
    std::function<void(void)> complete;

public:
    LoadingState(RWGame* game, std::function<void(void)> callback);

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(GameRenderer* r) override;

    bool shouldWorldUpdate() override;

    void handleEvent(const SDL_Event& e) override;
};

#endif  // LOADINGSTATE_HPP
