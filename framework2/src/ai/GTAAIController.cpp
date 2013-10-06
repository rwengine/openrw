#include "renderwure/ai/GTAAIController.hpp"
#include <renderwure/objects/GTACharacter.hpp>

GTAAIController::GTAAIController(GTACharacter* character)
: character(character) 
{
	character->controller = this;
}
