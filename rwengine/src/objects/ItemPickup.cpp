#include <objects/ItemPickup.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>
#include <items/WeaponItem.hpp>

ItemPickup::ItemPickup(GameWorld *world, const glm::vec3 &position, std::shared_ptr<WeaponData> weapon)
	: PickupObject(world, position, weapon->modelID), _data(weapon)
{

}

bool ItemPickup::onCharacterTouch(CharacterObject *character)
{
	character->addToInventory(new WeaponItem(_data));
	return true;
}
