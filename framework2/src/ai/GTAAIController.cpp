#include <ai/GTAAIController.hpp>
#include <objects/GTACharacter.hpp>

GTAAIController::GTAAIController(GTACharacter* character)
: character(character) 
{
	character->controller = this;
}
