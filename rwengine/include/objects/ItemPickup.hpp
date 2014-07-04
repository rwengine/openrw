#pragma once
#ifndef _ITEMPICKUP_HPP_
#define _ITEMPICKUP_HPP_
#include <objects/PickupObject.hpp>
#include <glm/glm.hpp>
#include <data/WeaponData.hpp>

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */
class ItemPickup : public PickupObject
{
	std::shared_ptr<WeaponData> _data;
public:

	ItemPickup(GameWorld* world, const glm::vec3& position, std::shared_ptr<WeaponData> weapon);

	bool onCharacterTouch(CharacterObject* character);
};

#endif
