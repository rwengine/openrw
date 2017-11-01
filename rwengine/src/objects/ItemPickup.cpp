#include "objects/ItemPickup.hpp"

#include "data/WeaponData.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/PickupObject.hpp"

ItemPickup::ItemPickup(GameWorld *world, const glm::vec3 &position,
                       BaseModelInfo *modelinfo, PickupType type,
                       WeaponData *item)
    : PickupObject(world, position, modelinfo, type), item(item) {
}

bool ItemPickup::onCharacterTouch(CharacterObject *character) {
    auto totalRounds = 0;

    switch (item->modelID) {
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

    if (getPickupType() == OnStreet || getPickupType() == OnStreetSlow) {
        totalRounds /= 5;
    }

    character->addToInventory(item->inventorySlot, totalRounds);

    return true;
}
