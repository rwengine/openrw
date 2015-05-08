#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "State.hpp"

class PlayerController;

class IngameState : public State
{
	enum CameraMode
	{
		CAMERA_CLOSE = 0,
		CAMERA_NORMAL = 1,
		CAMERA_FAR = 2,
		CAMERA_TOPDOWN = 3,
		/** Used for counting - not a valid camera mode */
		CAMERA_MAX
	};

	bool started;
	bool test;
	bool newgame;
	ViewCamera _look;
	/** Player input */
	glm::vec3 _movement;
	glm::vec3 cameraPosition;
	/** Timer to hold user camera position */
	float autolookTimer;
	CameraMode camMode;
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
