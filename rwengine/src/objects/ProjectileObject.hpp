#ifndef _RWENGINE_PROJECTILEOBJECT_HPP_
#define _RWENGINE_PROJECTILEOBJECT_HPP_

#include <objects/GameObject.hpp>
#include "render/VisualFX.hpp"


class GameWorld;
class btPairCachingGhostObject;
class btRigidBody;
class btSphereShape;
struct WeaponData;

/**
 * @brief Implements weapon projectile (e.g. molotovs, RPGs etc.)
 */
class ProjectileObject final : public GameObject {
public:
    enum ProjectileType {
        Grenade,
        Molotov,
        RPG,
    };

    struct ProjectileInfo {
        ProjectileInfo(ProjectileType type, glm::vec3 direction,
                       float velocity, float time, WeaponData* weapon)
            : type(type)
            , direction(direction)
            , velocity(velocity)
            , time(time)
            , weapon(weapon) {
        }
        ProjectileType type;
        glm::vec3 direction{};
        float velocity;

        /** Time to dentonation or removal */
        float time;

        WeaponData* weapon;
    };

private:
    ProjectileInfo _info;

    std::unique_ptr<btSphereShape> _shape;

    std::unique_ptr<btRigidBody> _body;

    /** Used for RPGs and Molotov collision detection */
    std::unique_ptr<btPairCachingGhostObject> _ghostBody;

    bool _exploded = false;

    void checkPhysicsContact();
    void explode();
    void cleanup();

public:
    /**
     * @brief ProjectileObject constructor
     */
    ProjectileObject(GameWorld* world, const glm::vec3& position,
                     const ProjectileInfo& info);

    ~ProjectileObject() override;

    void tick(float dt) override;

    Type type() const override {
        return Projectile;
    }

    const ProjectileInfo& getProjectileInfo() const {
        return _info;
    }
};

#endif
