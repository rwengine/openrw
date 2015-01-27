#include <objects/GenericPickup.hpp>

GenericPickup::GenericPickup(GameWorld* world, const glm::vec3& position, int modelID, int type)
: PickupObject(world, position, modelID), type(type)
{

}

bool GenericPickup::onCharacterTouch(CharacterObject* character)
{
	return true;
}
