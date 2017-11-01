#ifndef _RWENGINE_ITEMPICKUP_HPP_
#define _RWENGINE_ITEMPICKUP_HPP_

#include <glm/glm.hpp>

#include <objects/PickupObject.hpp>

class BaseModelInfo;
class CharacterObject;
class GameWorld;
struct WeaponData;

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */
class ItemPickup : public PickupObject {
    WeaponData* item;
public:
    ItemPickup(GameWorld* world, const glm::vec3& position,
               BaseModelInfo* modelinfo, PickupType type, WeaponData* item);

    bool onCharacterTouch(CharacterObject* character) override;
};

#endif
