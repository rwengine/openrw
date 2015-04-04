#pragma once
#ifndef _COLLISIONINSTANCE_HPP_
#define _COLLISIONINSTANCE_HPP_

#include <bullet/btBulletDynamicsCommon.h>
#include <vector>
#include <string>

class GameObject;
struct DynamicObjectData;
struct VehicleHandlingInfo;

/**
 * @brief Utility object for managing bullet objects.
 *
 * Stores handles to a btRigidBody and a set of collision shapes
 */
class CollisionInstance
{
public:

	CollisionInstance()
		: body(nullptr), vertArray(nullptr), motionState(nullptr), collisionHeight(0.f)
	{ }

	~CollisionInstance();

	bool createPhysicsBody(GameObject* object,
						   const std::string &modelName,
						   DynamicObjectData* dynamics = nullptr,
						   VehicleHandlingInfo* handling = nullptr);

	btRigidBody* body;
	std::vector<btCollisionShape*> shapes;
	btTriangleIndexVertexArray* vertArray;
	btMotionState* motionState;

	float collisionHeight;

};

#endif
