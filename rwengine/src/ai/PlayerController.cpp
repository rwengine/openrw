#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameWorld.hpp>
#include <glm/gtc/matrix_transform.hpp>

PlayerController::PlayerController(CharacterObject* character)
	: CharacterController(character), lastRotation(glm::vec3(0.f, 0.f, 0.f)), running(false)
{
	
}

void PlayerController::setRunning(bool run)
{
	running = run;
}

void PlayerController::updateCameraDirection(const glm::quat& rot)
{
	cameraRotation = rot;
}

void PlayerController::updateMovementDirection(const glm::vec3& dir, const glm::vec3 &rawdirection)
{
	direction = dir;
	_rawDirection = rawdirection;
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

	if( _currentActivity == nullptr ) {
		if( character->currentActivity != CharacterObject::Jump )
		{
			if( glm::length(direction) > 0.001f ) {
				character->enterAction(running ? CharacterObject::Run : CharacterObject::Walk);
			}
			else {
				character->enterAction(CharacterObject::Idle);
			}

			if( character->getCurrentVehicle() ) {
				character->getCurrentVehicle()->setSteeringAngle(_rawDirection.x);

				// TODO what is handbraking.
				character->getCurrentVehicle()->setThrottle(-_rawDirection.y);
			}
			else if( glm::length(direction) > 0.001f ) {
				character->rotation = cameraRotation * glm::quat(glm::vec3(0.f, 0.f, -atan2(direction.x, direction.y)));
			}
		}
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
		character->jump();
	}
}

