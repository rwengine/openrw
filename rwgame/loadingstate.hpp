#ifndef LOADINGSTATE_HPP
#define LOADINGSTATE_HPP

#include "State.hpp"

class LoadingState : public State
{
public:
	LoadingState(RWGame* game);

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void draw(GameRenderer* r);

	virtual void handleEvent(const sf::Event& event);
};

#endif // LOADINGSTATE_HPP
