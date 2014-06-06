#ifndef LOADINGSTATE_HHP
#define LOADINGSTATE_HPP

#include "State.hpp"

class LoadingState : public State
{
public:
	LoadingState();

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void draw(sf::RenderWindow &w);

	virtual void handleEvent(const sf::Event& event);
};

#endif // MENUSTATE_HPP
