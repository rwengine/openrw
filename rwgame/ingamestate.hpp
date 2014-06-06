#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "State.hpp"

#include <ai/PlayerController.hpp>

class IngameState : public State
{
	PlayerController* _player;
	CharacterObject* _playerCharacter;

	glm::vec2 _lookAngles;
	glm::vec3 _movement;
public:
	IngameState();

	void spawnPlayerVehicle();

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void handleEvent(const sf::Event& event);
};

#endif // INGAMESTATE_HPP
