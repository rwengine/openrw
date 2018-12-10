#ifndef _RWENGINE_WEAPONDATA_HPP_
#define _RWENGINE_WEAPONDATA_HPP_

#include <glm/vec3.hpp>

#include <cinttypes>
#include <string>

struct WeaponData {
    enum FireType { MELEE, INSTANT_HIT, PROJECTILE };

    std::string name;
    FireType fireType;
    float hitRange;
    int fireRate;
    int reloadMS;
    int clipSize;
    int damage;
    float speed;
    float meleeRadius;
    float lifeSpan;
    float spread;
    glm::vec3 fireOffset{};
    std::string animation1;
    std::string animation2;
    float animLoopStart;
    float animLoopEnd;
    float animFirePoint; /* Must be between 2 ^ */
    float animCrouchLoopStart;
    float animCrouchLoopEnd;
    float animCrouchFirePoint;
    float breakoutAnim;
    int modelID;
    std::uint32_t flags;

    std::uint32_t inventorySlot;
};

#endif
