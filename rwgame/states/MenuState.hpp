#ifndef MENUSTATE_HPP
#define MENUSTATE_HPP

#include "StateManager.hpp"

class MenuState : public State {
public:
    MenuState(RWGame* game);

    virtual void enter() override;
    virtual void exit() override;

    virtual void tick(float dt) override;

    virtual void enterMainMenu();
    virtual void enterLoadMenu();

    virtual void handleEvent(const SDL_Event& event) override;
};

#endif  // MENUSTATE_HPP
