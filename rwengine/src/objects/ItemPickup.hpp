#pragma once
#ifndef _ITEMPICKUP_HPP_
#define _ITEMPICKUP_HPP_
#include <glm/glm.hpp>
#include <objects/PickupObject.hpp>

class InventoryItem;

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */
class ItemPickup : public PickupObject {
    InventoryItem* item;

public:
    ItemPickup(GameWorld* world, const glm::vec3& position, PickupType type,
               InventoryItem* item);

    bool onCharacterTouch(CharacterObject* character);
};

#endif
