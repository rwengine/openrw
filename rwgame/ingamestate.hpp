#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "State.hpp"

class PlayerController;

class IngameState : public State
{
	ViewCamera _look;
	glm::vec2 _lookAngles;
	glm::vec3 _movement;
	float autolookTimer;
public:
	IngameState(RWGame* game, bool test = false);

	void startTest();
	void spawnPlayerVehicle();

	/** shortcut for getWorld()->state.player->getCharacter() */
	PlayerController* getPlayer();

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);
    virtual void draw(sf::RenderWindow& w);

	virtual void handleEvent(const sf::Event& event);

	const ViewCamera& getCamera();
};

#endif // INGAMESTATE_HPP
