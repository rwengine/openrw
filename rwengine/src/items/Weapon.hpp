#ifndef _RWENGINE_WEAPON_HPP_
#define _RWENGINE_WEAPON_HPP_
#include <glm/vec3.hpp>

class CharacterObject;
class GameObject;
struct WeaponData;

enum class ScanType {
    /** Instant-hit ray weapons */
    HitScan,
    /** Area of effect attack */
    Radius,
};

/**
 * @brief simple object for performing weapon checks against the world
 */
struct WeaponScan {
    const ScanType type;

    float damage;

    glm::vec3 center{};
    float radius;

    glm::vec3 end{};

    WeaponData* weapon;
    GameObject* source;

    // Constructor for Radius
    WeaponScan(float damage, const glm::vec3& center, float radius,
               WeaponData* weapon = nullptr, GameObject* source = nullptr)
            : type(ScanType::Radius)
            , damage(damage)
            , center(center)
            , radius(radius)
            , weapon(weapon)
            , source(source) {
    }

    // Constructor for HitScan
    WeaponScan(float damage, const glm::vec3& start, const glm::vec3& end,
               WeaponData* weapon = nullptr, GameObject* source = nullptr)
            : type(ScanType::HitScan)
            , damage(damage)
            , center(start)
            , end(end)
            , weapon(weapon)
            , source(source) {
    }

    bool doesDamage(GameObject* target) const;
};

namespace Weapon {
void fireProjectile(WeaponData* weapon, CharacterObject* character, float force);
void fireHitscan(WeaponData *weapon, CharacterObject* character);
void meleeHit(WeaponData *weapon, CharacterObject* character);
bool targetOnGround(WeaponData *weapon, CharacterObject* character);
}

#endif
