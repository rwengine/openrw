#pragma once
#ifndef _OBJECTINSTANCE_HPP_
#define _OBJECTINSTANCE_HPP_
#include <engine/GameObject.hpp>
#include <btBulletDynamicsCommon.h>

/**
 * @struct InstanceObject
 *  A simple object instance
 */
struct InstanceObject : public GameObject
{
	glm::vec3 scale;
	btRigidBody* body = nullptr;
	std::shared_ptr<ObjectData> object;
	std::shared_ptr<InstanceObject> LODinstance;
	std::shared_ptr<DynamicObjectData> dynamics;
	float _collisionHeight;

	InstanceObject(
		GameWorld* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		ModelHandle* model,
		const glm::vec3& scale,
		std::shared_ptr<ObjectData> obj,
		std::shared_ptr<InstanceObject> lod,
		std::shared_ptr<DynamicObjectData> dyn
			);

	Type type() { return Instance; }

	void tick(float dt);
	
	virtual bool takeDamage(const DamageInfo& damage);
};


#endif
