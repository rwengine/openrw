#include "items/Weapon.hpp"

#include <algorithm>

#include <glm/glm.hpp>

#include "data/WeaponData.hpp"
#include "engine/GameWorld.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/ProjectileObject.hpp"

void Weapon::fireHitscan(WeaponData* weapon, CharacterObject* owner) {
    auto handFrame = owner->getClump()->findFrame("srhand");
    glm::mat4 handMatrix = handFrame->getWorldTransform();

    const auto& raydirection = owner->getLookDirection();
    const auto rayend = owner->getPosition() + raydirection * weapon->hitRange;
    auto fireOrigin = glm::vec3(handMatrix[3]);
    auto dmg = static_cast<float>(weapon->damage);

    owner->engine->doWeaponScan({dmg, fireOrigin, rayend, weapon});
}

void Weapon::fireProjectile(WeaponData* weapon, CharacterObject* owner,
                            float force) {
    auto handPos = glm::vec3(0.f, 1.5f, 1.f);
    auto fireOrigin = owner->getPosition() + owner->getRotation() * handPos;
    auto direction =
        owner->getRotation() * glm::normalize(glm::vec3{0.f, 1.f, 1.f});

    auto pt = weapon->name == "grenade" ? ProjectileObject::Grenade
                                        : ProjectileObject::Molotov;

    force = std::max(0.1f, force);

    auto projectile = std::make_unique<ProjectileObject>(
        owner->engine, fireOrigin,
        ProjectileObject::ProjectileInfo{
            pt, direction,
            17.f * force,  /// @todo pull a better velocity from somewhere
            3.5f, weapon});
    auto ptr = projectile.get();

    auto& pool = owner->engine->getTypeObjectPool(ptr);
    pool.insert(std::move(projectile));
    owner->engine->allObjects.push_back(ptr);
}
