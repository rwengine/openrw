#pragma once
#ifndef _WEAPONITEM_HPP_
#define _WEAPONITEM_HPP_
#include <items/InventoryItem.hpp>
#include <data/WeaponData.hpp>
#include <memory>

class WeaponItem : public InventoryItem
{
	std::shared_ptr<WeaponData> _wepData;

	bool _firing;
public:
	WeaponItem(std::shared_ptr<WeaponData> data)
		: InventoryItem(data->inventorySlot, data->modelID), _wepData(data)
	{}

	void primary(CharacterObject* character, bool active);

	void secondary(CharacterObject* character, bool active);

	bool isFiring() const { return _firing; }

	std::shared_ptr<WeaponData>& getWeaponData() { return _wepData; }
};

#endif
