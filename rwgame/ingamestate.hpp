#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "State.hpp"

class PlayerController;

class IngameState : public State
{
	bool started;
	bool test;
	bool newgame;
	ViewCamera _look;
	/** Player input */
	glm::vec3 _movement;
	glm::vec3 cameraPosition;
	/** Timer to reset _lookAngles to forward in vehicles */
	float autolookTimer;
public:
	IngameState(RWGame* game, bool newgame = true, bool test = false);

	void startTest();
	void startGame();

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);
    virtual void draw(GameRenderer* r);

	virtual void handleEvent(const sf::Event& event);
	
	virtual bool shouldWorldUpdate();

	const ViewCamera& getCamera();
};

#endif // INGAMESTATE_HPP
