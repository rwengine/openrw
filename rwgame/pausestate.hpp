#ifndef PAUSESTATE_HPP
#define PAUSESTATE_HPP

#include "State.hpp"

class PauseState : public State
{
public:
	PauseState(RWGame* game);

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void handleEvent(const sf::Event& event);
};

#endif // PAUSESTATE_HPP
