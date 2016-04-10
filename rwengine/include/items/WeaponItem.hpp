#pragma once
#ifndef _WEAPONITEM_HPP_
#define _WEAPONITEM_HPP_
#include <items/InventoryItem.hpp>
#include <data/WeaponData.hpp>
#include <memory>

/**
 * @brief The WeaponItem class
 * Logic for basic weapon types
 *
 * This is instanciated once -per item type-, so state is shared between
 * all instances of the same weapon. Timing is controlled by the CharacterState
 */
class WeaponItem : public InventoryItem
{
	std::shared_ptr<WeaponData> _wepData;

	void fireHitscan(CharacterObject* owner);
	void fireProjectile(CharacterObject* owner);
public:
	WeaponItem(int itemID, std::shared_ptr<WeaponData> data)
		: InventoryItem(itemID, data->inventorySlot, data->modelID)
		, _wepData(data)
	{}

	void primary(CharacterObject* owner);

	void secondary(CharacterObject* owner);

	void fire(CharacterObject* owner);

	bool isFiring(CharacterObject* owner);

	std::shared_ptr<WeaponData>& getWeaponData() { return _wepData; }
};

#endif
