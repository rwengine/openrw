#pragma once
#ifndef _GTADEFAULTAICONTROLLER_HPP_
#define _GTADEFAULTAICONTROLLER_HPP_
#include "renderwure/ai/GTAAIController.hpp"

struct GTAAINode;
class GTADefaultAIController : public GTAAIController
{
	enum Action
	{
		Wander
	};
	
	Action action;
	
	GTAAINode* targetNode;
	
public:
	
    GTADefaultAIController(GTACharacter* character)
	 : GTAAIController(character), action(Wander), targetNode(nullptr) {}
	
    virtual void update(float dt);
	
    virtual glm::vec3 getTargetPosition();
	
    virtual glm::quat getTargetRotation();
};

#endif