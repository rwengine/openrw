#pragma once
#ifndef _GTADEFAULTAICONTROLLER_HPP_
#define _GTADEFAULTAICONTROLLER_HPP_
#include "ai/GTAAIController.hpp"
#include <random>

struct GTAAINode;
class GTADefaultAIController : public GTAAIController
{
public:
	
    GTADefaultAIController(CharacterObject* character)
	 : GTAAIController(character) {}

	glm::vec3 getTargetPosition();
};

#endif
