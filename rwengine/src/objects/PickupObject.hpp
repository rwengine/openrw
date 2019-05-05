#ifndef _RWENGINE_PICKUPOBJECT_HPP_
#define _RWENGINE_PICKUPOBJECT_HPP_

#include <cstdint>

#include <rw/debug.hpp>

#include <render/VisualFX.hpp>
#include <objects/GameObject.hpp>


class btPairCachingGhostObject;
class btSphereShape;

class BaseModelInfo;
class GameWorld;
class CharacterObject;
class VehicleObject;
struct VisualFX;

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
    enum BehaviourFlags { None = 0, PickupOnFoot = 1, PickupInVehicle = 2 };

    static bool defaultDoesRespawn(PickupType type);
    static float defaultRespawnTime(PickupType type);
    static BehaviourFlags defaultBehaviourFlags(PickupType type);

    PickupObject(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    ~PickupObject() override;

    Type type() const override {
        return Pickup;
    }

    void tick(float dt) override;

    virtual bool onPlayerTouch() {
        return false;
    }

    virtual bool onPlayerVehicleTouch() {
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

    virtual bool isBigNVeinyPickup() const {
        return false;
    }

    bool doesRespawn() const {
        return respawn;
    }

    void setRespawn(bool r) {
        respawn = r;
    }

    float getRespawnTime() const {
        return respawnTime;
    }

    void setRespawnTime(float time) {
        respawnTime = time;
    }

    BehaviourFlags getBehaviourFlags() const {
        return behaviourFlags;
    }

    void setBehaviourFlags(BehaviourFlags flags) {
        behaviourFlags = flags;
    }

private:
    std::unique_ptr<btPairCachingGhostObject> m_ghost;
    std::unique_ptr<btSphereShape> m_shape;
    bool m_enabled = false;
    float m_enableTimer = 0.f;
    bool m_collected = false;
    ParticleFX& m_corona;
    short m_colourId = 0;
    bool respawn = false;
    float respawnTime{};
    BehaviourFlags behaviourFlags{};

    PickupType m_type{};
};

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */

struct WeaponData;

class ItemPickup : public PickupObject {
    const WeaponData& item;

public:
    ItemPickup(GameWorld* world, const glm::vec3& position,
               BaseModelInfo* modelinfo, PickupType type, const WeaponData& p_item);

    bool onPlayerTouch() override;
};

/**
 * @brief The DummyPickup class
 */
class DummyPickup : public PickupObject {
public:
    DummyPickup(GameWorld* world, const glm::vec3& position,
                BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
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

    bool onPlayerTouch() override;
};

/**
 * @brief The HealthPickup class
 */
class HealthPickup : public PickupObject {
public:
    HealthPickup(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The ArmourPickup class
 */
class ArmourPickup : public PickupObject {
public:
    ArmourPickup(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The CollectablePickup class
 */
class CollectablePickup : public PickupObject {
public:
    CollectablePickup(GameWorld* world, const glm::vec3& position,
                      BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The AdrenalinePickup class
 */
class AdrenalinePickup : public PickupObject {
public:
    AdrenalinePickup(GameWorld* world, const glm::vec3& position,
                     BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
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

    bool onPlayerTouch() override;
};


/**
 * @brief The BigNVeinyPickup class
 */
class BigNVeinyPickup : public PickupObject {
public:
    BigNVeinyPickup(GameWorld* world, const glm::vec3& position,
                    BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerVehicleTouch() override;

    bool isBigNVeinyPickup() const override {
        return true;
    }

    static const std::array<glm::vec3, 106>& getBigNVeinyPickupsLocations();
};

#endif
