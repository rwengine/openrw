#pragma once
#ifndef _PLAYERCONTROLLER_HPP_
#define _PLAYERCONTROLLER_HPP_
#include <ai/CharacterController.hpp>

class PlayerController : public CharacterController
{
	glm::quat cameraRotation;
	
	glm::vec3 direction;

	glm::quat lastRotation;

	bool _enabled;
	
public:
	
	PlayerController(CharacterObject* character);

	/**
	 * @brief Enables and disables player input.
	 * @todo actually implement input being disabled.
	 */
	void setInputEnabled(bool enabled);
	bool isInputEnabled() const;
	
	void updateCameraDirection(const glm::quat& rot);
	
	void updateMovementDirection(const glm::vec3& pos, const glm::vec3& rawdirection);
	
	void exitVehicle();
	
	void enterNearestVehicle();
	
    virtual void update(float dt);
	
    virtual glm::vec3 getTargetPosition();
	
	void jump();

	/**
	 * returns 0 (only one player supported)
	 */
	int getScriptObjectID() const { return 0; }
};

#endif
