#ifndef MENUSTATE_HPP
#define MENUSTATE_HPP

#include "State.hpp"

class MenuState final : public State {
public:
    MenuState(RWGame* game);

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    virtual void enterMainMenu();
    virtual void enterLoadMenu();

    void handleEvent(const SDL_Event& event) override;
};

#endif  // MENUSTATE_HPP
