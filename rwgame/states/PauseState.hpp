#ifndef PAUSESTATE_HPP
#define PAUSESTATE_HPP

#include <SDL2/SDL_events.h>
#include "State.hpp"

class PauseState : public State
{
public:
	PauseState(RWGame* game);

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);
	
    virtual void draw(GameRenderer* r);

	virtual void handleEvent(const SDL_Event& event);
};

#endif // PAUSESTATE_HPP
