#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "State.hpp"

class PlayerController;

class IngameState : public State
{
	ViewCamera _look;
	glm::vec2 _lookAngles;
	glm::vec3 _movement;
public:
	IngameState(bool test = false);

	void startTest();
	void spawnPlayerVehicle();

	void updateView();

	/** shortcut for getWorld()->state.player->getCharacter() */
	PlayerController* getPlayer();

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void handleEvent(const sf::Event& event);

	const ViewCamera& getCamera();
};

#endif // INGAMESTATE_HPP
