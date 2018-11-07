#ifndef _RWENGINE_WEAPON_HPP_
#define _RWENGINE_WEAPON_HPP_

class CharacterObject;
struct WeaponData;

namespace Weapon {
void fireProjectile(WeaponData* weapon, CharacterObject* character, float force);
void fireHitscan(WeaponData *weapon, CharacterObject* character);
}

#endif
