#include "data/CollisionInstance.hpp"

#include <engine/GameObject.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>

CollisionInstance::~CollisionInstance()
{
	if( body ) {
		GameObject* object = static_cast<GameObject*>(body->getUserPointer());

		// Remove body from existance.
		object->engine->dynamicsWorld->removeRigidBody(body);
		
		for(btCollisionShape* shape : shapes) {
			delete shape;
		}
		
		delete body;
	}
	if( vertArray ) {
		delete vertArray;
	}
	if( motionState ) {
		delete motionState;
	}
}

bool CollisionInstance::createPhysicsBody(GameObject *object, const std::string& modelName, DynamicObjectData *dynamics, VehicleHandlingInfo *handling)
{
	auto phyit = object->engine->gameData.collisions.find(modelName);
	if( phyit != object->engine->gameData.collisions.end()) {
		btCompoundShape* cmpShape = new btCompoundShape;

		auto p = object->getPosition();
		auto r = object->getRotation();

		motionState = new btDefaultMotionState;
		motionState->setWorldTransform(btTransform(
									btQuaternion(r.x, r.y, r.z, -r.w).inverse(),
									btVector3(p.x, p.y, p.z)
									));
		shapes.push_back(cmpShape);

		btRigidBody::btRigidBodyConstructionInfo info(0.f, motionState, cmpShape);

		CollisionModel& physInst = *phyit->second.get();

		float colMin = std::numeric_limits<float>::max(),
				colMax = std::numeric_limits<float>::lowest();

		// Boxes
		for( size_t i = 0; i < physInst.boxes.size(); ++i ) {
			auto& box = physInst.boxes[i];
			auto size = (box.max - box.min) / 2.f;
			auto mid = (box.min + box.max) / 2.f;
			btCollisionShape* bshape = new btBoxShape( btVector3(size.x, size.y, size.z)  );
			btTransform t; t.setIdentity();
			t.setOrigin(btVector3(mid.x, mid.y, mid.z));
			cmpShape->addChildShape(t, bshape);

			colMin = std::min(colMin, mid.z - size.z);
			colMax = std::max(colMax, mid.z + size.z);

			shapes.push_back(bshape);
		}

		// Spheres
		for( size_t i = 0; i < physInst.spheres.size(); ++i ) {
			auto& sphere = physInst.spheres[i];
			btCollisionShape* sshape = new btSphereShape(sphere.radius);
			btTransform t; t.setIdentity();
			t.setOrigin(btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
			cmpShape->addChildShape(t, sshape);

			colMin = std::min(colMin, sphere.center.z - sphere.radius);
			colMax = std::max(colMax, sphere.center.z + sphere.radius);

			shapes.push_back(sshape);
		}

		if( physInst.vertices.size() > 0 && physInst.indices.size() >= 3 ) {
			vertArray = new btTriangleIndexVertexArray(
						physInst.indices.size()/3,
						(int*) physInst.indices.data(),
						sizeof(uint32_t)*3,
						physInst.vertices.size(),
						&(physInst.vertices[0].x),
					sizeof(glm::vec3));
			btBvhTriangleMeshShape* trishape = new btBvhTriangleMeshShape(vertArray, false);
			trishape->setMargin(0.05f);
			btTransform t; t.setIdentity();
			cmpShape->addChildShape(t, trishape);

			shapes.push_back(trishape);
		}

		collisionHeight = colMax - colMin;

		if( dynamics ) {
			if( dynamics->uprootForce > 0.f ) {
				info.m_mass = 0.f;
			}
			else {
				btVector3 inert;
				cmpShape->calculateLocalInertia(dynamics->mass, inert);
				info.m_mass = dynamics->mass;
				info.m_localInertia = inert;
			}
		}
		else if( handling ) {
			btVector3 inert;
			cmpShape->calculateLocalInertia(handling->mass, inert);
			info.m_mass = handling->mass;
			info.m_localInertia = inert;
		}

		body = new btRigidBody(info);
		body->setUserPointer(object);
		object->engine->dynamicsWorld->addRigidBody(body);

		if( dynamics && dynamics->uprootForce > 0.f ) {
			body->setCollisionFlags(body->getCollisionFlags()
									| btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}
	}
	else
	{
		return false;
	}

	return true;
}
