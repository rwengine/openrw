#include <objects/InstanceObject.hpp>
#include <engine/GameWorld.hpp>
#include <data/CollisionModel.hpp>
#include <dynamics/CollisionInstance.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>

InstanceObject::InstanceObject(GameWorld* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		const ModelRef& model,
		const glm::vec3& scale,
		std::shared_ptr<ObjectData> obj,
		InstanceObject* lod,
		std::shared_ptr<DynamicObjectData> dyn)
: GameObject(engine, pos, rot, model), scale(scale), body(nullptr), object(obj),
  LODinstance(lod), dynamics(dyn), _enablePhysics(false)
{
	if( obj ) {
		changeModel(obj);

		for( auto& path : obj->paths )
		{
			engine->aigraph.createPathNodes(position, rot, path);
		}
	}

}

InstanceObject::~InstanceObject()
{
	if( body ) {
		delete body;
	}
}

void InstanceObject::tick(float dt)
{
	if( dynamics && body ) {
		if( body->body->isStaticObject() ) {
			if( _enablePhysics ) {
				// Apparently bodies must be removed and re-added if their mass changes.
				engine->dynamicsWorld->removeRigidBody(body->body);
				btVector3 inert;
				body->body->getCollisionShape()->calculateLocalInertia(dynamics->mass, inert);
				body->body->setMassProps(dynamics->mass, inert);
				engine->dynamicsWorld->addRigidBody(body->body);
			}
		}
		
		_updateLastTransform();

		auto _bws = body->body->getWorldTransform().getOrigin();
		glm::vec3 ws(_bws.x(), _bws.y(), _bws.z());
		auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		float vH = ws.z;// - _collisionHeight/2.f;
		float wH = 0.f;

		if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
			int i = (wX*WATER_HQ_DATA_SIZE) + wY;
			int hI = engine->data->realWater[i];
			if( hI < NO_WATER_INDEX ) {
				wH = engine->data->waterHeights[hI];
				wH += engine->data->getWaveHeightAt(ws);
				if( vH <= wH ) {
					inWater = true;
				}
				else {
					inWater = false;
				}
			}
			else {
				inWater = false;
			}
		}
		_lastHeight = ws.z;

		if( inWater ) {
			float oZ = -(body->collisionHeight * (dynamics->bouancy/100.f));
			body->body->activate(true);
			// Damper motion
			body->body->setDamping(0.95f, 0.9f);

			auto wi = engine->data->getWaterIndexAt(ws);
			if(wi != NO_WATER_INDEX) {
				float h = engine->data->waterHeights[wi] + oZ;

				// Calculate wave height
				h += engine->data->getWaveHeightAt(ws);

				if ( ws.z <= h ) {
					/*if( dynamics->uprootForce > 0.f && (body->body->getCollisionFlags() & btRigidBody::CF_STATIC_OBJECT) != 0 ) {
						// Apparently bodies must be removed and re-added if their mass changes.
						engine->dynamicsWorld->removeRigidBody(body->body);
						btVector3 inert;
						body->getCollisionShape()->calculateLocalInertia(dynamics->mass, inert);
						body->setMassProps(dynamics->mass, inert);
						engine->dynamicsWorld->addRigidBody(body);
					}*/

					float x = (h - ws.z);
					float F = WATER_BUOYANCY_K * x + -WATER_BUOYANCY_C * body->body->getLinearVelocity().z();
					btVector3 forcePos = btVector3(0.f, 0.f, 2.f).rotate(
								body->body->getOrientation().getAxis(), body->body->getOrientation().getAngle());
					body->body->applyForce(btVector3(0.f, 0.f, F),
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
		delete body;
		body = nullptr;
	}

	object = incoming;

	if( incoming ) {
		auto bod = new CollisionInstance;

		if( bod->createPhysicsBody(this, object->modelName, dynamics.get()) )
		{
			bod->body->setActivationState(ISLAND_SLEEPING);
			body = bod;
		}
	}
}

glm::vec3 InstanceObject::getPosition() const
{
	if( body ) {
		btVector3 Pos = body->body->getWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z());
	}
	return position;
}

glm::quat InstanceObject::getRotation() const
{
	if( body ) {
		btQuaternion rot = body->body->getWorldTransform().getRotation();
		return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
	}
	return rotation;
}

void InstanceObject::setRotation(const glm::quat &r)
{
	if( body ) {
		auto& wtr = body->body->getWorldTransform();
		wtr.setRotation(btQuaternion(r.x, r.y, r.z, r.w));
	}
	GameObject::setRotation(r);
}

bool InstanceObject::takeDamage(const GameObject::DamageInfo& dmg)
{
	RW_CHECK(dmg.hitpoints == 0, "Instance damange not implemented yet");
	bool explodeOnHit = (object->flags&ObjectData::EXPLODEONHIT) == ObjectData::EXPLODEONHIT;
	bool smash = (object->flags&ObjectData::SMASHABLE) == ObjectData::SMASHABLE;
	if( dynamics ) {
		smash = dynamics->collDamageFlags == 80;

		if( dmg.impulse >= dynamics->uprootForce && (body->body->getCollisionFlags() & btRigidBody::CF_STATIC_OBJECT) != 0 ) {
			_enablePhysics = true;
		}
	}
	if(explodeOnHit || smash)
	{
		if(explodeOnHit) {
			// explode
			//mHealth = -1.f;
		}
		else {
			//mHealth -= dmg.hitpoints;
		}
		return true;
	}
	return false;
}


