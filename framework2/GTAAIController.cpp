#include "renderwure/ai/GTAAIController.hpp"
#include <renderwure/engine/GTAObjects.hpp>

GTAAIController::GTAAIController(GTACharacter* character)
: character(character) 
{
	character->controller = this;
}