#pragma once
#ifndef _GTADEFAULTAICONTROLLER_HPP_
#define _GTADEFAULTAICONTROLLER_HPP_
#include "renderwure/ai/GTAAIController.hpp"
#include <random>

struct GTAAINode;
class GTADefaultAIController : public GTAAIController
{
	enum Action
	{
		Wander
	};
	
	Action action;
	
	GTAAINode* targetNode;
	GTAAINode* lastNode;
	
	float nodeMargin; /// Minimum distance away to "reach" node.
	float getUpTime; /// Time to wait before getting up.
	
public:
	
    GTADefaultAIController(GTACharacter* character)
	 : GTAAIController(character),
	   action(Wander),
	   targetNode(nullptr),
	   lastNode(nullptr),
	   nodeMargin(0.f), getUpTime(-1.f) {}
	
	void update(float dt);
	
	glm::vec3 getTargetPosition();
};

#endif
