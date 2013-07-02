#pragma once
#ifndef _LOADERCOL_HPP_
#define _LOADERCOL_HPP_

#include <string>
#include <vector>
#include <glm/glm.hpp>

typedef glm::vec3 CollTVec3;

struct CollTBounds
{
	CollTVec3 min, max;
	CollTVec3 center;
	float radius;
};

struct CollTBoundsV1
{
	float radius;
	CollTVec3 center;
	CollTVec3 min, max;
};

struct CollTSurface
{
	uint8_t material;
	uint8_t flag;
	uint8_t brightness;
	uint8_t light;
};

struct CollTSphere
{
	CollTVec3 center;
	float radius;
	CollTSurface surface;
};

struct CollTSphereV1
{
	float radius;
	CollTVec3 center;
	CollTSurface surface;
};

struct CollTBox
{
	CollTVec3 min, max;
	CollTSurface surface;
};

struct CollTFaceGroup
{
	CollTVec3 min, max;
	uint16_t startface, endface;
};

typedef glm::vec3 CollTVertex;

struct CollTFace
{
	uint16_t a, b, c;
	uint8_t material;
	uint8_t light;
};

struct CollTFaceV1
{
	uint32_t a, b, c;
	CollTSurface surface;
};

struct CollTHeader
{
	char name[22];
	uint16_t modelid;
	CollTBounds bounds;
};

struct CollTHeaderV2
{
	uint16_t numspheres;
	uint16_t numboxes;
	uint32_t numfaces;
	uint32_t flags;
	uint32_t offsetspheres;
	uint32_t offsetboxes;
	uint32_t offsetlines;
	uint32_t offsetverts;
	uint32_t offsetfaces;
};

struct CollTHeaderV3
{
	uint32_t numshadowfaces;
	uint32_t offsetverts;
	uint32_t offsetfaces;
};

/**
 * @class CollisionInstance
 * Stores data about a collision proxy
 */
class CollisionInstance
{
	
};

/**
 * @class LoaderCOL
 *  Loads collision data from COL files
 */
class LoaderCOL
{
public:
	/// Load the COL data into memory
	bool load(const std::string& file);
	
	std::vector<CollisionInstance> instances;
};

#endif