#ifndef DEBUGSTATE_HPP
#define DEBUGSTATE_HPP

#include "State.hpp"

class DebugState : public State
{
	glm::vec3 _debugPos;
	glm::vec3 _movement;
	glm::vec2 _debugAngles;
	bool _freeLook;
	bool _sonicMode;
public:
	DebugState();

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void handleEvent(const sf::Event& event);

	void spawnVehicle(unsigned int id);
};

#endif // DEBUGSTATE_HPP
