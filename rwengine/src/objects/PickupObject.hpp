#pragma once
#ifndef _PICKUPOBJECT_HPP_
#define _PICKUPOBJECT_HPP_
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletCollisionCommon.h>
#include <glm/glm.hpp>
#include <objects/GameObject.hpp>

class VisualFX;
class CharacterObject;

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

    PickupObject(GameWorld* world, const glm::vec3& position, int modelID,
                 PickupType type);

    ~PickupObject();

    int getModelID() const {
        return m_model;
    }

    Type type() {
        return Pickup;
    }

    void tick(float dt);

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

    PickupType getPickupType() const {
        return m_type;
    }

private:
    btPairCachingGhostObject* m_ghost;
    btSphereShape* m_shape;
    bool m_enabled;
    float m_enableTimer;
    bool m_collected;
    int m_model;
    VisualFX* m_corona;

    PickupType m_type;
};

#endif
