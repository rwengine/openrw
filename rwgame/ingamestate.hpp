#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "State.hpp"

#include <ai/GTAAIController.hpp>
#include <ai/GTAPlayerAIController.hpp>

class IngameState : public State
{
	GTAPlayerAIController* _player;
	GTACharacter* _playerCharacter;

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
