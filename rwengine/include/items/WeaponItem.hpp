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
	float _fireStart;
	float _fireStop;

	void fireHitscan();
	void fireProjectile();
public:
	WeaponItem(CharacterObject* character, std::shared_ptr<WeaponData> data)
		: InventoryItem(data->inventorySlot, data->modelID, character), _wepData(data),
		  _firing(false), _fireStart(0.f), _fireStop(0.f)
	{}

	void primary(bool active);

	void secondary(bool active);

	bool isFiring() const { return _firing; }

	void fire();

	std::shared_ptr<WeaponData>& getWeaponData() { return _wepData; }
};

#endif
