#pragma once
#ifndef _INVENTORYITEM_HPP_
#define _INVENTORYITEM_HPP_

class GameObject;
class CharacterObject;

/**
 * @brief The InventoryItem class
 *
 * Instanciated once per item type
 */
class InventoryItem
{
  int _itemID;
  int _inventorySlot;
  int _modelID;

protected:
  InventoryItem(int itemID, int invSlot, int model)
      : _itemID(itemID), _inventorySlot(invSlot), _modelID(model)
  {
  }

public:
  virtual ~InventoryItem() {}

  /**
   * @brief getObject
   * @return The ID of the model associated with the item.
   */
  int getModelID() { return _modelID; }

  /**
    * @brief getItemID
    * @return The index of this item in the item list
    */
  int getItemID() { return _itemID; }

  /**
   * @brief getInventorySlot
   * @return The inventory slot number for this item
   */
  int getInventorySlot() const { return _inventorySlot; }

  /**
   * @brief primary Implements mouse 1 action
   * @param owner The character using this item
   */
  virtual void primary(CharacterObject* owner) = 0;

  /**
   * @see primary
   * @param owner The character using this item
   */
  virtual void secondary(CharacterObject* owner) = 0;

  constexpr static int NO_INVSLOT = -1;
};

#endif
