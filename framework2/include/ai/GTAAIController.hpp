#pragma once
#ifndef _GTAAICONTROLLER_HPP_
#define _GTAAICONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct GTACharacter;
/**
 * @class GTAAIController
 * Character Controller Interface, translates high-level behaviours into low level actions.
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
	
	/**
	 * @brief update Updates the controller.
	 * @param dt
	 */
	virtual void update(float dt) = 0;

	virtual glm::vec3 getTargetPosition() = 0;
};

#endif
