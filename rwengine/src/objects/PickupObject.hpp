#ifndef _RWENGINE_PICKUPOBJECT_HPP_
#define _RWENGINE_PICKUPOBJECT_HPP_

#include <cstdint>
#include <glm/glm.hpp>

#include <rw/defines.hpp>

#include <objects/GameObject.hpp>

class btPairCachingGhostObject;
class btSphereShape;

class BaseModelInfo;
class GameWorld;
class CharacterObject;
class VisualFX;

/**
 * @brief The PickupObject class
 * Implements interface and base behaviour for pickups
 */
class PickupObject : public GameObject {
public:
    enum PickupType {
        InShop = 1,
        OnStreet = 2,
        Once = 3,
        OnceTimeout = 4,
        Collectable = 5,
        OutOfStock = 6,
        Money = 7,
        MineInactive = 8,
        MineArmed = 9,
        NauticalMineInactive = 10,
        NauticalMineArmed = 11,
        FloatingPackage = 12,
        FloatingPackageFloating = 13,
        OnStreetSlow = 14
    };
    enum /*BehaviourFlags*/ { PickupOnFoot = 1, PickupInVehicle = 2 };

    static bool doesRespawn(PickupType type);
    static float respawnTime(PickupType type);
    static uint32_t behaviourFlags(PickupType type);

    PickupObject(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    ~PickupObject() override;

    Type type() const override {
        return Pickup;
    }

    void tick(float dt) override;

    virtual bool onCharacterTouch(CharacterObject* character) {
        RW_UNUSED(character);
        return false;
    }

    bool isEnabled() const {
        return m_enabled;
    }
    void setEnabled(bool enabled);

    bool isCollected() const {
        return m_collected;
    }

    void setCollected(bool collected) {
        m_collected = collected;
    }

    PickupType getPickupType() const {
        return m_type;
    }

    virtual bool isRampage() const {
        return false;
    }

private:
    btPairCachingGhostObject* m_ghost;
    btSphereShape* m_shape;
    bool m_enabled;
    float m_enableTimer;
    bool m_collected;
    VisualFX* m_corona;
    short m_colourId;

    PickupType m_type;
};

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */

struct WeaponData;

class ItemPickup : public PickupObject {
    WeaponData* item;

public:
    ItemPickup(GameWorld* world, const glm::vec3& position,
               BaseModelInfo* modelinfo, PickupType type, WeaponData* item);

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The DummyPickup class
 */
class DummyPickup : public PickupObject {
public:
    DummyPickup(GameWorld* world, const glm::vec3& position,
                BaseModelInfo* modelinfo, PickupType type);

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The RampagePickup class
 */
class RampagePickup : public PickupObject {
public:
    RampagePickup(GameWorld* world, const glm::vec3& position,
                  BaseModelInfo* modelinfo, PickupType type);

    bool isRampage() const override {
        return true;
    }

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The HealthPickup class
 */
class HealthPickup : public PickupObject {
public:
    HealthPickup(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The ArmourPickup class
 */
class ArmourPickup : public PickupObject {
public:
    ArmourPickup(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The CollectablePickup class
 */
class CollectablePickup : public PickupObject {
public:
    CollectablePickup(GameWorld* world, const glm::vec3& position,
                      BaseModelInfo* modelinfo, PickupType type);

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The AdrenalinePickup class
 */
class AdrenalinePickup : public PickupObject {
public:
    AdrenalinePickup(GameWorld* world, const glm::vec3& position,
                     BaseModelInfo* modelinfo, PickupType type);

    bool onCharacterTouch(CharacterObject* character) override;
};

/**
 * @brief The MoneyPickup class
 */
class MoneyPickup : public PickupObject {
    uint32_t money;

public:
    MoneyPickup(GameWorld* world, const glm::vec3& position,
                BaseModelInfo* modelinfo, PickupType type, uint32_t money);

    void setMoney(uint32_t m) {
        money = m;
    };

    bool onCharacterTouch(CharacterObject* character) override;
};

#endif
