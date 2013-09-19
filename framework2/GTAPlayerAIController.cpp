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
		character->getCurrentVehicle()->setSteeringAngle(-direction.x * 3.131f);

		// TODO what is handbraking.
		character->getCurrentVehicle()->setThrottle(direction.y);
	}
	else if( glm::length(direction) > 0.001f ) {
		character->rotation = cameraRotation * glm::quat(glm::vec3(0.f, 0.f, -atan2(direction.x, direction.y)));
	}

}

glm::vec3 GTAPlayerAIController::getTargetPosition()
{
	return direction;
}
