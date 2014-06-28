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
};

/**
 * @brief The WeaponScan struct
 * Represents a scene query against a ray
 * or shape used to determine what to damage.
 */
struct WeaponScan
{
	enum ScanType {
		HITSCAN,
		RADIUS,
	};

	const ScanType type;

	float damage;

	glm::vec3 center;
	float radius;

	glm::vec3 end;

	// Constructor for a RADIUS hitscan
	WeaponScan( float damage, const glm::vec3& center, float radius )
		: type(RADIUS), damage(damage), center(center), radius(radius)
	{}

	// Constructor for a ray hitscan
	WeaponScan( float damage, const glm::vec3& start, const glm::vec3& end)
		: type(HITSCAN), damage(damage), center(start), end(end)
	{}
};

#endif
