#ifndef RWENGINE_WEAPON_HPP
#define RWENGINE_WEAPON_HPP

#include <data/WeaponData.hpp>
#include <objects/CharacterObject.hpp>

namespace Weapon {
void fireProjectile(WeaponData* wepon, CharacterObject* character, float force);
void fireHitscan(WeaponData* wepon, CharacterObject* character);
}

#endif
