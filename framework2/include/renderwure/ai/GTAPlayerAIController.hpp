#pragma once
#ifndef _GTAPLAYERAICONTROLLER_HPP_
#define _GTAPLAYERAICONTROLLER_HPP_
#include "renderwure/ai/GTAAIController.hpp"

class GTAPlayerAIController : public GTAAIController
{
	
	glm::quat cameraRotation;
	
	glm::vec3 direction;
	
	glm::quat lastRotation;
	
	bool running;
	
public:
	
    GTAPlayerAIController(GTACharacter* character);
	
	void setRunning(bool run);
	
	void updateCameraDirection(const glm::quat& rot);
	
	void updateMovementDirection(const glm::vec3& pos);
	
    virtual void update(float dt);
	
    virtual glm::vec3 getTargetPosition();
};

#endif
