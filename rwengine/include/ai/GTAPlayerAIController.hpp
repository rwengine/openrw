#pragma once
#ifndef _GTAPLAYERAICONTROLLER_HPP_
#define _GTAPLAYERAICONTROLLER_HPP_
#include "ai/GTAAIController.hpp"

class GTAPlayerAIController : public GTAAIController
{
	
	glm::quat cameraRotation;
	
	glm::vec3 direction;
	glm::vec3 _rawDirection;
	
	glm::quat lastRotation;
	
	bool running;
	
public:
	
    GTAPlayerAIController(GTACharacter* character);
	
	void setRunning(bool run);
	
	void updateCameraDirection(const glm::quat& rot);
	
	void updateMovementDirection(const glm::vec3& pos, const glm::vec3& rawdirection);
	
	void exitVehicle();
	
	void enterNearestVehicle();
	
    virtual void update(float dt);
	
    virtual glm::vec3 getTargetPosition();
	
	void jump();
};

#endif
