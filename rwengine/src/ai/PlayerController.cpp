#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameWorld.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/Animator.hpp>

PlayerController::PlayerController(CharacterObject* character)
	: CharacterController(character), lastRotation(glm::vec3(0.f, 0.f, 0.f)), _enabled(true)
{
	
}

void PlayerController::setInputEnabled(bool enabled)
{
	_enabled = enabled;
}

bool PlayerController::isInputEnabled() const
{
	return _enabled;
}

void PlayerController::updateCameraDirection(const glm::quat& rot)
{
	cameraRotation = rot;
}

void PlayerController::updateMovementDirection(const glm::vec3& dir, const glm::vec3 &rawdirection)
{
	if( _currentActivity == nullptr ) {
		direction = dir;
		setRawMovement(rawdirection);
	}
}

void PlayerController::exitVehicle()
{
	if(character->getCurrentVehicle()) {
		setNextActivity(new Activities::ExitVehicle());
	}
}

void PlayerController::enterNearestVehicle()
{
	if(! character->getCurrentVehicle()) {
		auto world = character->engine;
		VehicleObject* nearest = nullptr; float d = 10.f;

		for( GameObject* object : world->objects ) {
			if( object->type() == GameObject::Vehicle ) {
				float vd = glm::length( character->getPosition() - object->getPosition());
				if( vd < d ) {
					d = vd;
					nearest = static_cast<VehicleObject*>(object);
				}
			}
		}

		if( nearest ) {
			setNextActivity(new Activities::EnterVehicle(nearest, 0));
		}
	}
}

void PlayerController::update(float dt)
{
	// TODO: Determine if the player is allowed to interupt the current activity.
	if( glm::length(direction) > 0.001f && _currentActivity != nullptr ) {
		skipActivity();
	}

	if( _currentActivity == nullptr && glm::length(rawMovement) > 0.0f ) {
		character->rotation = glm::quat(glm::vec3(0.f, 0.f, -atan2(direction.x, direction.y)));
	}

	CharacterController::update(dt);
}

glm::vec3 PlayerController::getTargetPosition()
{
	return direction;
}

void PlayerController::jump()
{
	if(! character->isInWater() ) {
		setNextActivity(new Activities::Jump());
	}
}

