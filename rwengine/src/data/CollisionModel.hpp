#ifndef RWENGINE_COLLISIONMODEL_HPP
#define RWENGINE_COLLISIONMODEL_HPP
#include <glm/glm.hpp>
#include <string>
#include <cstdint>
#include <vector>

/**
 * @class CollisionModel 
 * Collision shapes data container.
 */
struct CollisionModel
{
	/// @todo give shapes surface data
	struct Sphere 
	{
		glm::vec3 center;
		float radius;
	};
	
	struct Box
	{
		glm::vec3 min;
		glm::vec3 max;
	};
	
	/// COL version
	int version;
	
	/// Model name
	std::string name;
	uint16_t modelid;
	
	// Bounding radius
	float radius;
	
	glm::vec3 center;
	glm::vec3 min;
	glm::vec3 max;
	
	std::vector<Sphere> spheres;
	std::vector<Box> boxes;
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;
};

#endif
