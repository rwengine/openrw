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

const float followRadius = 5.f;

void DefaultAIController::update(float dt)
{
	switch(currentGoal)
	{
		case FollowLeader:
		{
			if( ! leader ) break;
			if( getCharacter()->getCurrentVehicle() )
			{
				if( leader->getCurrentVehicle() != getCharacter()->getCurrentVehicle() )
				{
					skipActivity();
					setNextActivity(new Activities::ExitVehicle);
				}
				// else we're already in the right spot.
			}
			else
			{
				if( leader->getCurrentVehicle() )
				{
					setNextActivity(new Activities::EnterVehicle(leader->getCurrentVehicle(), 1));
				}
				else
				{
					glm::vec3 dir = leader->getPosition() - getCharacter()->getPosition();
					if( glm::length(dir) > followRadius )
					{
						if( glm::distance(gotoPos, leader->getPosition()) > followRadius )
						{
							gotoPos = leader->getPosition() + ( glm::normalize(-dir) * followRadius * 0.7f );
							skipActivity();
							setNextActivity(new Activities::GoTo(gotoPos));
						}
					}
				}
			}
		}
		break;
		default: break;
	}
	
	CharacterController::update(dt);
}
