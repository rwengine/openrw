#ifndef DEBUGSTATE_HPP
#define DEBUGSTATE_HPP

#include "State.hpp"

class DebugState : public State
{
	ViewCamera _debugCam;
	glm::vec3 _movement;
	glm::vec2 _debugLook;
	bool _freeLook;
	bool _sonicMode;
public:
	DebugState(const glm::vec3& vp = {}, const glm::quat& vd = {});

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void handleEvent(const sf::Event& event);

	void spawnVehicle(unsigned int id);

	const ViewCamera& getCamera();
};

#endif // DEBUGSTATE_HPP
