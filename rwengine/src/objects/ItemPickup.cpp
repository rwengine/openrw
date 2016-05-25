#include <objects/ItemPickup.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>
#include <items/WeaponItem.hpp>
#include <rw/defines.hpp>

ItemPickup::ItemPickup(GameWorld *world, const glm::vec3 &position, PickupType type, InventoryItem* item)
	: PickupObject(world, position, item->getModelID(), type)
	, item(item)
{
	RW_CHECK(item != nullptr, "Pickup created with null item");
}

bool ItemPickup::onCharacterTouch(CharacterObject *character)
{
	character->addToInventory(item);
	return true;
}
