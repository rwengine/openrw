#ifndef _RWENGINE_PROJECTILEOBJECT_HPP_
#define _RWENGINE_PROJECTILEOBJECT_HPP_

#include <glm/glm.hpp>

#include <objects/GameObject.hpp>

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
        ProjectileType type;
        glm::vec3 direction{};
        float velocity;

        /** Time to dentonation or removal */
        float time;

        WeaponData* weapon;
    };

private:
    ProjectileInfo _info;

    btSphereShape* _shape;

    btRigidBody* _body = nullptr;

    /** Used for RPGs and Molotov collision detection */
    btPairCachingGhostObject* _ghostBody = nullptr;

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
