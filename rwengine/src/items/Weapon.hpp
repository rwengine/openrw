#ifndef _RWENGINE_WEAPON_HPP_
#define _RWENGINE_WEAPON_HPP_
#include <glm/glm.hpp>

class CharacterObject;
class GameObject;
struct WeaponData;

/**
 * @brief simple object for performing weapon checks against the world
 */
struct WeaponScan {
    enum ScanType {
        /** Instant-hit ray weapons */
        HITSCAN,
        /** Area of effect attack */
        RADIUS,
    };

    const ScanType type;

    float damage;

    glm::vec3 center{};
    float radius;

    glm::vec3 end{};

    WeaponData* weapon;
    GameObject* source;

    // Constructor for a RADIUS hitscan
    WeaponScan(float damage, const glm::vec3& center, float radius,
               WeaponData* weapon = nullptr, GameObject* source = nullptr)
            : type(RADIUS)
            , damage(damage)
            , center(center)
            , radius(radius)
            , weapon(weapon)
            , source(source) {
    }

    // Constructor for a ray hitscan
    WeaponScan(float damage, const glm::vec3& start, const glm::vec3& end,
               WeaponData* weapon = nullptr, GameObject* source = nullptr)
            : type(HITSCAN)
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
}

#endif
