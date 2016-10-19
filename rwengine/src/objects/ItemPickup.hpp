#pragma once
#ifndef _ITEMPICKUP_HPP_
#define _ITEMPICKUP_HPP_
#include <data/WeaponData.hpp>
#include <glm/glm.hpp>
#include <objects/PickupObject.hpp>

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */
class ItemPickup : public PickupObject {
    WeaponData* item;
public:
    ItemPickup(GameWorld* world, const glm::vec3& position,
               BaseModelInfo* modelinfo, PickupType type, WeaponData* item);

    bool onCharacterTouch(CharacterObject* character);
};

#endif
