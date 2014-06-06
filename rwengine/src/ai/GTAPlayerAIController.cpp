#include "ai/GTAPlayerAIController.hpp"
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameWorld.hpp>
#include <glm/gtc/matrix_transform.hpp>

GTAPlayerAIController::GTAPlayerAIController(CharacterObject* character)
	: GTAAIController(character), lastRotation(glm::vec3(0.f, 0.f, 0.f)), running(false)
{
	
}

void GTAPlayerAIController::setRunning(bool run)
{
	running = run;
}

void GTAPlayerAIController::updateCameraDirection(const glm::quat& rot)
{
	cameraRotation = rot;
}

void GTAPlayerAIController::updateMovementDirection(const glm::vec3& dir, const glm::vec3 &rawdirection)
{
	direction = dir;
	_rawDirection = rawdirection;
}

void GTAPlayerAIController::exitVehicle()
{
	if(character->getCurrentVehicle()) {
		setNextActivity(new Activities::ExitVehicle());
	}
}

void GTAPlayerAIController::enterNearestVehicle()
{
	if(! character->getCurrentVehicle()) {
		auto world = character->engine;
		VehicleObject* nearest = nullptr; float d = 10.f;
		for(auto it = world->vehicleInstances.begin(); it != world->vehicleInstances.end(); ++it) {
			float vd = glm::length( character->getPosition() - (*it)->getPosition());
			if( vd < d ) { 
				d = vd;
				nearest = *it;
			}
		}
		
		if( nearest ) {
			setNextActivity(new Activities::EnterVehicle(nearest, 0));
		}
	}
}

void GTAPlayerAIController::update(float dt)
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

	GTAAIController::update(dt);
}

glm::vec3 GTAPlayerAIController::getTargetPosition()
{
	return direction;
}

void GTAPlayerAIController::jump()
{
	character->jump();
}

