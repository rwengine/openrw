#include "renderwure/ai/GTAPlayerAIController.hpp"
#include <renderwure/engine/GTAObjects.hpp>

GTAPlayerAIController::GTAPlayerAIController(GTACharacter* character)
 : GTAAIController(character), running(false)
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
}

glm::vec3 GTAPlayerAIController::getTargetPosition()
{
	return glm::vec3();
}

glm::quat GTAPlayerAIController::getTargetRotation()
{
	return cameraRotation * glm::quat(glm::vec3(0.f, 0.f, -atan2(direction.x, direction.y)));
}