#include "renderwure/ai/GTADefaultAIController.hpp"
#include <renderwure/engine/GTAObjects.hpp>
#include <renderwure/engine/GTAEngine.hpp>

void GTADefaultAIController::update(float dt)
{
	if( action == Wander ) {
		if( targetNode == nullptr ) {
			float d = std::numeric_limits< float >::max();
			auto oldTarget = targetNode;
			for( auto n = character->engine->ainodes.begin();
				n != character->engine->ainodes.end();
				++n ) {
				if( n->type != GTAAINode::Pedestrian ) continue;
				float ld = glm::length( n->position - character->position );
				if( ld > 2.5f && ld < d ) {
					d = ld;
					targetNode = &(*n);
				}
			}
			
			if( targetNode == nullptr ) {
				character->changeAction(GTACharacter::Idle);
			}
			else {
				character->changeAction(GTACharacter::Walk);
				// Choose a new random margin
				std::uniform_real_distribution<float> dist(1.f, 2.5f);
				nodeMargin = dist(character->engine->randomEngine);
			}
		}
		else if( glm::length(targetNode->position - character->position) < nodeMargin ) {
			if( targetNode->nextIndex >= 0 && targetNode->nextIndex < character->engine->ainodes.size() ) {
				targetNode = &character->engine->ainodes[targetNode->nextIndex];
			}
			else {
				targetNode = nullptr;
			}
		}
	}
}

glm::vec3 GTADefaultAIController::getTargetPosition()
{
	return glm::vec3();
}

glm::quat GTADefaultAIController::getTargetRotation()
{
	if( action == Wander && targetNode != nullptr ) {
		auto d = targetNode->position - character->position;
		float a = atan2( d.x, d.y );
		return glm::quat( glm::vec3(0.f, 0.f, -a) );
	}
	return glm::quat();
}
