#pragma once
#ifndef _INVENTORYITEM_HPP_
#define _INVENTORYITEM_HPP_

class GameObject;
class CharacterObject;

class InventoryItem
{
	int _inventorySlot;
	int _modelID;
protected:
	InventoryItem(int invSlot, int model)
		: _inventorySlot(invSlot), _modelID(model)
	{}
public:

	/**
	 * @brief getObject
	 * @return The ID of the model associated with the item.
	 */
	int getModelID() { return _modelID; }

	/**
	 * @brief getInventorySlot
	 * @return The inventory slot number for this item
	 */
	int getInventorySlot() const { return _inventorySlot; }

	/**
	 * @brief primary Implements mouse 1 action
	 * @param active action starting or ending
	 */
	virtual void primary(CharacterObject* character, bool active) = 0;

	/**
	 * @see primary
	 */
	virtual void secondary(CharacterObject* character, bool active) = 0;

	constexpr static int NO_INVSLOT = -1;
};

#endif
