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
	auto& wep = character->getCurrentState().weapons[item->getInventorySlot()];
	auto totalRounds = 0, clipRounds = 0;

	switch (item->getModelID()) {
	case 173: /* Pistol */
		totalRounds = 45;
		break;
	case 178: /* Uzi */
		totalRounds = 125;
		break;
	case 176: /* Shotgun */
		totalRounds = 25;
		break;
	case 170: /* Grenade */
		totalRounds = 5;
		break;
	case 174: /* Molotov */
		totalRounds = 5;
		break;
	case 181: /* Flame thrower */
		totalRounds = 25;
		break;
	case 171: /* AK */
		totalRounds = 150;
		break;
	case 180: /* M16 */
		totalRounds = 300;
		break;
	case 177: /* Sniper Rifle */
		totalRounds = 25;
		break;
	}

	if (getPickupType() == OnStreet || getPickupType() == OnStreetSlow)
	{
		totalRounds /= 5;
	}

	wep.bulletsTotal = totalRounds;
	wep.bulletsClip = clipRounds;

	return true;
}
