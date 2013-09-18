#include "renderwure/ai/GTAPlayerAIController.hpp"
#include <renderwure/objects/GTACharacter.hpp>
#include <renderwure/objects/GTAVehicle.hpp>

GTAPlayerAIController::GTAPlayerAIController(GTACharacter* character)
	: GTAAIController(character), running(false), lastRotation(glm::vec3(0.f, 0.f, 0.f))
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

void GTAPlayerAIController::updateMovementDirection(const glm::vec3& dir)
{
	direction = dir;
}

void GTAPlayerAIController::update(float dt)
{
	if( glm::length(direction) > 0.001f ) {
		character->changeAction(running ? GTACharacter::Run : GTACharacter::Walk);
	}
	else {
		character->changeAction(GTACharacter::Idle);
	}

	if( character->getCurrentVehicle() ) {
		glm::vec3 dir = glm::length(direction) > 0.001f ? direction : glm::vec3(0.f, 1.f, 0.f);
		glm::vec3 vehicleDir = character->getCurrentVehicle()->getRotation() * dir;
		float a = -atan2(vehicleDir.x, vehicleDir.y);
		character->getCurrentVehicle()->setSteeringAngle(a);
	}
	else if( glm::length(direction) > 0.001f ) {
		character->rotation = cameraRotation * glm::quat(glm::vec3(0.f, 0.f, -atan2(direction.x, direction.y)));
	}

	// TODO what is handbraking.
	if( direction.y > 0.f ) {
		character->getCurrentVehicle()->setThrottle(direction.y);
	}
	else if( direction.y < 0.f ) {
		character->getCurrentVehicle()->setBraking(-direction.y);
	}
}

glm::vec3 GTAPlayerAIController::getTargetPosition()
{
	return direction;
}
