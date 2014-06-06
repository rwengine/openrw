#include <ai/DefaultAIController.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>

glm::vec3 DefaultAIController::getTargetPosition()
{
	/*if(targetNode) {
		if(lastNode && character->getCurrentVehicle()) {
			auto nDir = glm::normalize(targetNode->position - lastNode->position);
			auto right = glm::cross(nDir, glm::vec3(0.f, 0.f, 1.f));
			return targetNode->position + right * 2.2f;
		}
		return targetNode->position;
	}*/
	return glm::vec3();
}
