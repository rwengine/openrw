#include <objects/InstanceObject.hpp>
#include <engine/GameWorld.hpp>
#include <data/CollisionModel.hpp>
#include <engine/Animator.hpp>

InstanceObject::InstanceObject(GameWorld* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		ModelHandle *model,
		const glm::vec3& scale,
		std::shared_ptr<ObjectData> obj,
		InstanceObject* lod,
		std::shared_ptr<DynamicObjectData> dyn)
: GameObject(engine, pos, rot, model), scale(scale), body(nullptr), object(obj),
  LODinstance(lod), dynamics(dyn), _collisionHeight(0.f), _enablePhysics(false)
{
	if( obj ) {
		changeModel(obj);

		auto pathit = engine->objectNodes.find(obj->ID);
		if( pathit != engine->objectNodes.end() ) {
			auto& pathlist = pathit->second;
			for( size_t p = 0; p < pathlist.size(); ++p ) {
				auto& path = pathlist[p];
				engine->aigraph.createPathNodes(position, rot, *path);
			}
		}
	}

}

InstanceObject::~InstanceObject()
{
	if( body ) {
		engine->dynamicsWorld->removeRigidBody(body);
		delete body;
	}
}

void InstanceObject::tick(float dt)
{
	if( dynamics && body ) {
		if( body->isStaticObject() ) {
			if( _enablePhysics ) {
				// Apparently bodies must be removed and re-added if their mass changes.
				engine->dynamicsWorld->removeRigidBody(body);
				btVector3 inert;
				body->getCollisionShape()->calculateLocalInertia(dynamics->mass, inert);
				body->setMassProps(dynamics->mass, inert);
				engine->dynamicsWorld->addRigidBody(body);
			}
		}

		auto _bws = body->getWorldTransform().getOrigin();
		glm::vec3 ws(_bws.x(), _bws.y(), _bws.z());
		auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		float vH = ws.z;// - _collisionHeight/2.f;
		float wH = 0.f;

		if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
			int i = (wX*WATER_HQ_DATA_SIZE) + wY;
			int hI = engine->gameData.realWater[i];
			if( hI < NO_WATER_INDEX ) {
				wH = engine->gameData.waterHeights[hI];
				wH += engine->gameData.getWaveHeightAt(ws);
				if( vH <= wH ) {
					_inWater = true;
				}
				else {
					_inWater = false;
				}
			}
			else {
				_inWater = false;
			}
		}
		_lastHeight = ws.z;

		if( _inWater ) {
			float oZ = -(_collisionHeight * (dynamics->bouancy/100.f));
			body->activate(true);
			// Damper motion
			body->setDamping(0.95f, 0.9f);

			auto wi = engine->gameData.getWaterIndexAt(ws);
			if(wi != NO_WATER_INDEX) {
				float h = engine->gameData.waterHeights[wi] + oZ;

				// Calculate wave height
				h += engine->gameData.getWaveHeightAt(ws);

				if ( ws.z <= h ) {
					if( dynamics->uprootForce > 0.f && (body->getCollisionFlags() & btRigidBody::CF_STATIC_OBJECT) != 0 ) {
						// Apparently bodies must be removed and re-added if their mass changes.
						engine->dynamicsWorld->removeRigidBody(body);
						btVector3 inert;
						body->getCollisionShape()->calculateLocalInertia(dynamics->mass, inert);
						body->setMassProps(dynamics->mass, inert);
						engine->dynamicsWorld->addRigidBody(body);
					}

					float x = (h - ws.z);
					float F = WATER_BUOYANCY_K * x + -WATER_BUOYANCY_C * body->getLinearVelocity().z();
					btVector3 forcePos = btVector3(0.f, 0.f, 2.f).rotate(
								body->getOrientation().getAxis(), body->getOrientation().getAngle());
					body->applyForce(btVector3(0.f, 0.f, F),
										 forcePos);
				}
			}
		}
	}

	if( animator ) animator->tick(dt);
}

void InstanceObject::changeModel(std::shared_ptr<ObjectData> incoming)
{
	if( body ) {
		engine->dynamicsWorld->removeRigidBody(body);
		delete body;
	}

	object = incoming;

	if( incoming ) {
		auto phyit = engine->gameData.collisions.find(object->modelName);
		if( phyit != engine->gameData.collisions.end()) {
			btCompoundShape* cmpShape = new btCompoundShape;
			btDefaultMotionState* msta = new btDefaultMotionState;
			msta->setWorldTransform(btTransform(
										btQuaternion(
											rotation.x, rotation.y, rotation.z, -rotation.w
											).inverse(),
										btVector3(
											position.x, position.y, position.z
											)
										));
			btRigidBody::btRigidBodyConstructionInfo info(0.f, msta, cmpShape);
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
			}

			if( physInst.vertices.size() > 0 && physInst.indices.size() >= 3 ) {
				btTriangleIndexVertexArray* vertarray = new btTriangleIndexVertexArray(
							physInst.indices.size()/3,
							(int*) physInst.indices.data(),
							sizeof(uint32_t)*3,
							physInst.vertices.size(),
							&(physInst.vertices[0].x),
						sizeof(glm::vec3));
				btBvhTriangleMeshShape* trishape = new btBvhTriangleMeshShape(vertarray, false);
				trishape->setMargin(0.05f);
				btTransform t; t.setIdentity();
				cmpShape->addChildShape(t, trishape);
			}

			_collisionHeight = colMax - colMin;

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

			body = new btRigidBody(info);
			body->setUserPointer(this);
			engine->dynamicsWorld->addRigidBody(body);

			if( dynamics && dynamics->uprootForce > 0.f ) {
				body->setCollisionFlags(body->getCollisionFlags()
										| btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
			}

			body->setActivationState(ISLAND_SLEEPING);
		}
	}
}

void InstanceObject::setRotation(const glm::quat &r)
{
	if( body ) {
		auto wtr = body->getWorldTransform();
		wtr.setRotation(btQuaternion(r.x, r.y, r.z, r.w));
	}
	GameObject::setRotation(r);
}

bool InstanceObject::takeDamage(const GameObject::DamageInfo& dmg)
{
	bool explodeOnHit = (object->flags&ObjectData::EXPLODEONHIT) == ObjectData::EXPLODEONHIT;
	bool smash = (object->flags&ObjectData::SMASHABLE) == ObjectData::SMASHABLE;
	if( dynamics ) {
		smash = dynamics->collDamageFlags == 80;

		if( dmg.impulse >= dynamics->uprootForce && (body->getCollisionFlags() & btRigidBody::CF_STATIC_OBJECT) != 0 ) {
			_enablePhysics = true;
		}
	}
	if(explodeOnHit || smash)
	{
		if(explodeOnHit) {
			// explode
			mHealth = -1.f;
		}
		else {
			mHealth -= dmg.hitpoints;
		}
		return true;
	}
	return false;
}


