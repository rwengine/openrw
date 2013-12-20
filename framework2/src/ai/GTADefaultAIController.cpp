#include "ai/GTADefaultAIController.hpp"
#include <objects/GTACharacter.hpp>
#include <engine/GTAEngine.hpp>
#include <engine/Animator.hpp>
#include <objects/GTAVehicle.hpp>

void GTADefaultAIController::update(float dt)
{
	if( character->currentActivity == GTACharacter::KnockedDown ) {
		if( getUpTime < 0.f ) {
			getUpTime = 1.5f;
		}
		else {
			getUpTime -= dt;
			if(getUpTime < 0.f) {
				character->changeAction(GTACharacter::GettingUp);
				getUpTime = -1.f;
			}
		}
	}
	else if (character->currentActivity == GTACharacter::GettingUp ) {
		if( character->animator->isCompleted() ) {
			character->changeAction(GTACharacter::Idle);
		}
	}
	else {
		if( action == Wander ) {
			if( targetNode == nullptr ) {
				GTAAINode::NodeType currentType = character->getCurrentVehicle()
						? GTAAINode::Vehicle : GTAAINode::Pedestrian;

				float d = std::numeric_limits< float >::max();
				for( auto n = character->engine->aigraph.nodes.begin();
					n != character->engine->aigraph.nodes.end();
					++n ) {
					if( (*n)->type != currentType ) continue;
					if( (*n) == lastNode ) continue;
					float ld = glm::length( (*n)->position - character->getPosition() );
					if( ld > nodeMargin && ld < d ) {
						d = ld;
						targetNode = (*n);
					}
				}
				
				if( targetNode == nullptr ) {
					character->changeAction(GTACharacter::Idle);
				}
				else {
					character->changeAction(GTACharacter::Walk);
					// Choose a new random margin
					if(character->getCurrentVehicle()) {
						std::uniform_real_distribution<float> dist(2.f, 2.5f);
						nodeMargin = dist(character->engine->randomEngine);
					}
					else {
						std::uniform_real_distribution<float> dist(1.f, 1.5f);
						nodeMargin = dist(character->engine->randomEngine);
					}
				}
			}
			else if( glm::length(getTargetPosition() - character->getPosition()) < nodeMargin ) {
				if(targetNode->connections.size() > 0) {
					std::uniform_int_distribution<int> uniform(0, targetNode->connections.size()-1);
					GTAAINode* nextNode = nullptr; size_t i = 0;
					do
					{
						nextNode = targetNode->connections[uniform(character->engine->randomEngine)];
					} while(i++ < targetNode->connections.size() && nextNode == lastNode);
					lastNode = targetNode;
					targetNode = nextNode;
				}
				else {
					targetNode = nullptr;
					character->changeAction(GTACharacter::Idle);
				}
			}
			else 
			{
				// Ensure the AI doesn't get stuck in idle with a target node.
				if(character->currentActivity == GTACharacter::Idle) {
					character->changeAction(GTACharacter::Walk);
				}
			}
		}

		if( action == Wander && targetNode != nullptr ) {
			auto d = targetNode->position - character->getPosition();

			if(lastNode && character->getCurrentVehicle()) {
				auto nDir = glm::normalize(targetNode->position - lastNode->position);
				auto right = glm::cross(nDir, glm::vec3(0.f, 0.f, 1.f));
				d += right * 2.2f;
			}

			if(character->getCurrentVehicle()) {
				auto vd = glm::inverse(character->getCurrentVehicle()->getRotation()) * d;
				float va = -atan2( vd.x, vd.y );
				if(glm::abs(va) > (3.141f/2.f)) {
					va = -va;
				}
				character->getCurrentVehicle()->setSteeringAngle(va);
			}
			else {
				float a = -atan2( d.x, d.y );
				character->rotation = glm::quat(glm::vec3(0.f, 0.f, a));
			}
		}

		if(character->getCurrentVehicle()) {
			if(targetNode == nullptr) {
				character->getCurrentVehicle()->setThrottle(0.f);
			}
			else {
				float targetVelocity = 5.f;
				float perc = (targetVelocity - character->getCurrentVehicle()->physBody->getLinearVelocity().length())/targetVelocity;
				perc = std::min(1.f, std::max(0.f, perc));

				// Determine if the vehicle should reverse instead.
				auto td = getTargetPosition() - character->getPosition();
				auto vd = character->getCurrentVehicle()->getRotation() * glm::vec3(0.f, 1.f, 0.f);
				if(glm::dot(td, vd) < -0.25f) {
					perc *= -1.f;
				}

				character->getCurrentVehicle()->setThrottle(perc);
			}
		}
	}
}

glm::vec3 GTADefaultAIController::getTargetPosition()
{
	if(targetNode) {
		if(lastNode && character->getCurrentVehicle()) {
			auto nDir = glm::normalize(targetNode->position - lastNode->position);
			auto right = glm::cross(nDir, glm::vec3(0.f, 0.f, 1.f));
			return targetNode->position + right * 2.2f;
		}
		return targetNode->position;
	}
	return glm::vec3();
}
