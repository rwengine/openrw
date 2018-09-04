#ifndef _RWENGINE_PROJECTILEOBJECT_HPP_
#define _RWENGINE_PROJECTILEOBJECT_HPP_

#include <glm/glm.hpp>

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
        ProjectileInfo(ProjectileType p_type, glm::vec3 p_direction,
                       float p_velocity, float p_time, WeaponData* p_weapon)
            : type(p_type)
            , direction(p_direction)
            , velocity(p_velocity)
            , time(p_time)
            , weapon(p_weapon) {
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
