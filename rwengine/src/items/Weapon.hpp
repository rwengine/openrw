#ifndef _RWENGINE_WEAPON_HPP_
#define _RWENGINE_WEAPON_HPP_

class CharacterObject;
struct WeaponData;

namespace Weapon {
void fireProjectile(WeaponData* wepon, CharacterObject* character, float force);
void fireHitscan(WeaponData* wepon, CharacterObject* character);
}

#endif
