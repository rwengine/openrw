#pragma once
#ifndef _OBJECTINSTANCE_HPP_
#define _OBJECTINSTANCE_HPP_
#include <engine/GameObject.hpp>

/**
 * @struct InstanceObject
 *  A simple object instance
 */
struct InstanceObject : public GameObject
{
	glm::vec3 scale;
	std::shared_ptr<ObjectData> object;
	std::shared_ptr<InstanceObject> LODinstance;

	InstanceObject(
		GameWorld* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		ModelHandle* model,
		const glm::vec3& scale,
		std::shared_ptr<ObjectData> obj,
		std::shared_ptr<InstanceObject> lod
			);

	Type type() { return Instance; }
	
    virtual bool takeDamage(const DamageInfo& damage);
};


#endif
