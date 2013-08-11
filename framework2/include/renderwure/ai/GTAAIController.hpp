#pragma once
#ifndef _GTAAICONTROLLER_HPP_
#define _GTAAICONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct GTACharacter;
/**
 * @class GTAAIController
 * Interface for controlling a Pedestrian, telling it which way to face and 
 * what actions to perform.
 */
class GTAAIController
{
protected:
	
	/**
	 * The character being controlled.
	 */
	GTACharacter* character;
	
public:
	
	GTAAIController(GTACharacter* character);
	
	virtual void update(float dt) = 0;
	
	/**
	 * @return The position the pedestrian should move towards
	 */
	virtual glm::vec3 getTargetPosition() = 0;
	
	/**
	 * @return The direction the pedestrian should be facing.
	 */
	virtual glm::quat getTargetRotation() = 0;
};

#endif