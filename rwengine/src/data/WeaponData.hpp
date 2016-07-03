#pragma once
#ifndef _WEAPONDATA_HPP_
#define _WEAPONDATA_HPP_
#include <string>
#include <glm/glm.hpp>
#include <cinttypes>

struct WeaponData
{
	enum FireType {
		MELEE,
		INSTANT_HIT,
		PROJECTILE
	};

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
	glm::vec3 fireOffset;
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

	int inventorySlot;
};

/**
 * @brief simple object for performing weapon checks against the world
 *
 * @todo RADIUS hitscans
 */
struct WeaponScan
{
	enum ScanType {
		/** Instant-hit ray weapons */
		HITSCAN,
		/** Area of effect attack */
		RADIUS,
	};

	const ScanType type;

	float damage;

	glm::vec3 center;
	float radius;

	glm::vec3 end;

	WeaponData* weapon;

	// Constructor for a RADIUS hitscan
	WeaponScan( float damage, const glm::vec3& center, float radius, WeaponData* weapon = nullptr )
		: type(RADIUS), damage(damage), center(center), radius(radius), weapon(weapon)
	{}

	// Constructor for a ray hitscan
	WeaponScan( float damage, const glm::vec3& start, const glm::vec3& end, WeaponData* weapon = nullptr )
		: type(HITSCAN), damage(damage), center(start), end(end), weapon(weapon)
	{}
};

#endif
