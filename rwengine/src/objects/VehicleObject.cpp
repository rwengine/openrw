#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <sys/stat.h>
#include <data/CollisionModel.hpp>
#include <render/Model.hpp>

VehicleObject::VehicleObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, ModelHandle* model, VehicleDataHandle data, VehicleInfoHandle info, const glm::vec3& prim, const glm::vec3& sec)
	: GameObject(engine, pos, rot, model),
	  steerAngle(0.f), throttle(0.f), brake(0.f), handbrake(false),
	  damageFlags(0), vehicle(data), info(info), colourPrimary(prim),
	  colourSecondary(sec), physBody(nullptr), physVehicle(nullptr)
{
	mHealth = 100.f;
	if(! data->modelName.empty()) {
		auto phyit = engine->gameData.collisions.find(data->modelName);
		if( phyit != engine->gameData.collisions.end()) {
			btCompoundShape* cmpShape = new btCompoundShape;
			btDefaultMotionState* msta = new btDefaultMotionState;
			msta->setWorldTransform(btTransform(
				btQuaternion(
					rot.x, rot.y, rot.z, rot.w
				),
				btVector3(
					pos.x, pos.y, pos.z
				)
			));
			CollisionModel& physInst = *phyit->second.get();

			btVector3 com(info->handling.centerOfMass.x, info->handling.centerOfMass.y, info->handling.centerOfMass.z);

			// Boxes
			for( size_t i = 0; i < physInst.boxes.size(); ++i ) {
				auto& box = physInst.boxes[i];
				auto size = (box.max - box.min) / 2.f;
				auto mid = (box.min + box.max) / 2.f;
				btCollisionShape* bshape = new btBoxShape( btVector3(size.x, size.y, size.z)  );
				btTransform t; t.setIdentity();
				t.setOrigin(btVector3(mid.x, mid.y, mid.z));
				cmpShape->addChildShape(t, bshape);
			}

			// Spheres
			for( size_t i = 0; i < physInst.spheres.size(); ++i ) {
				auto& sphere = physInst.spheres[i];
				btCollisionShape* sshape = new btSphereShape(sphere.radius/2.f);
				btTransform t; t.setIdentity();
				t.setOrigin(btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
				cmpShape->addChildShape(t, sshape);
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
				trishape->setMargin(0.09f);
				btTransform t; t.setIdentity();
				cmpShape->addChildShape(t, trishape);
			}

			btVector3 inertia(0,0,0);
			cmpShape->calculateLocalInertia(info->handling.mass, inertia);

			btRigidBody::btRigidBodyConstructionInfo rginfo(info->handling.mass, msta, cmpShape, inertia);

			physBody = new btRigidBody(rginfo);
			physBody->setUserPointer(this);
			engine->dynamicsWorld->addRigidBody(physBody);

			physRaycaster = new VehicleRaycaster(this, engine->dynamicsWorld);
			btRaycastVehicle::btVehicleTuning tuning;

			float travel = fabs(info->handling.suspensionUpperLimit - info->handling.suspensionLowerLimit);
			tuning.m_frictionSlip = 1.8f;
			tuning.m_maxSuspensionTravelCm = travel * 100.f;

			physVehicle = new btRaycastVehicle(tuning, physBody, physRaycaster);
			physVehicle->setCoordinateSystem(0, 2, 1);
			physBody->setActivationState(DISABLE_DEACTIVATION);
			engine->dynamicsWorld->addVehicle(physVehicle);

			float kC = 0.4f;
			float kR = 0.6f;

			for(size_t w = 0; w < info->wheels.size(); ++w) {
				auto restLength = travel;
				auto heightOffset = info->handling.suspensionUpperLimit;
				btVector3 connection(
							info->wheels[w].position.x,
							info->wheels[w].position.y,
							info->wheels[w].position.z + heightOffset );
				bool front = connection.y() > 0;
				btWheelInfo& wi = physVehicle->addWheel(connection, btVector3(0.f, 0.f, -1.f), btVector3(1.f, 0.f, 0.f), restLength, data->wheelScale / 2.f, tuning, front);
				wi.m_suspensionRestLength1 = restLength;

				wi.m_maxSuspensionForce = 100000.f;
				wi.m_suspensionStiffness = (info->handling.suspensionForce * 10.f);

				//float dampEffect = (info->handling.suspensionDamping) / travel;
				//wi.m_wheelsDampingCompression = wi.m_wheelsDampingRelaxation = dampEffect;

				wi.m_wheelsDampingCompression = kC * 2.f * btSqrt(wi.m_suspensionStiffness);
				wi.m_wheelsDampingRelaxation = kR * 2.f * btSqrt(wi.m_suspensionStiffness);
				wi.m_rollInfluence = 0.0f;
				wi.m_frictionSlip = tuning.m_frictionSlip * (front ? info->handling.tractionBias : 1.f - info->handling.tractionBias);
			}

		}
	}
}

VehicleObject::~VehicleObject()
{
	engine->dynamicsWorld->removeRigidBody(physBody);
	engine->dynamicsWorld->removeVehicle(physVehicle);
	delete physBody;
	delete physVehicle;
	delete physRaycaster;
	
	ejectAll();
}

void VehicleObject::setPosition(const glm::vec3& pos)
{
	GameObject::setPosition(pos);
	if(physBody) {
		auto t = physBody->getWorldTransform();
		t.setOrigin(btVector3(pos.x, pos.y, pos.z));
		physBody->setWorldTransform(t);
	}
}

glm::vec3 VehicleObject::getPosition() const
{
	if(physVehicle) {
		btVector3 Pos = physVehicle->getChassisWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z());
	}
	return position;
}

glm::quat VehicleObject::getRotation() const
{
	if(physVehicle) {
		btQuaternion rot = physVehicle->getChassisWorldTransform().getRotation();
		return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
	}
	return rotation;
}

#include <glm/gtc/type_ptr.hpp>

void VehicleObject::tick(float dt)
{
	if(physVehicle) {
		// todo: a real engine function
		float velFac = (info->handling.maxVelocity - physVehicle->getCurrentSpeedKmHour()) / info->handling.maxVelocity;
		float engineForce = info->handling.acceleration * 150.f * throttle * velFac;

		for(int w = 0; w < physVehicle->getNumWheels(); ++w) {
			btWheelInfo& wi = physVehicle->getWheelInfo(w);
			if( info->handling.driveType == VehicleHandlingInfo::All ||
					(info->handling.driveType == VehicleHandlingInfo::Forward && wi.m_bIsFrontWheel) ||
					(info->handling.driveType == VehicleHandlingInfo::Rear && !wi.m_bIsFrontWheel))
			{
					physVehicle->applyEngineForce(engineForce, w);
			}

			float brakeReal = info->handling.brakeDeceleration * info->handling.mass * (wi.m_bIsFrontWheel? info->handling.brakeBias : 1.f - info->handling.brakeBias);
			physVehicle->setBrake(brakeReal * brake, w);

			if(wi.m_bIsFrontWheel) {
				float sign = std::signbit(steerAngle) ? -1.f : 1.f;
				physVehicle->setSteeringValue(std::min(info->handling.steeringLock*(3.141f/180.f), std::abs(steerAngle)) * sign, w);
				//physVehicle->setSteeringValue(std::min(3.141f/2.f, std::abs(steerAngle)) * sign, w);
			}
		}

		if( vehicle->type == VehicleData::BOAT ) {
			if( isInWater() ) {
				btVector3 localSteer = btVector3(info->handling.dimensions.x * steerAngle, 0.f, 0.f)
						.rotate(physBody->getOrientation().getAxis(), physBody->getOrientation().getAngle());
				physBody->applyForce(
							physVehicle->getForwardVector() * engineForce * 100.f,
							localSteer);
			}
			physBody->setDamping(0.2f, 0.6f);
		}

		auto ws = getPosition();
		auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		float vH = ws.z - info->handling.dimensions.z;
		float wH = 0.f;

		if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
			int i = (wX*WATER_HQ_DATA_SIZE) + wY;
			int hI = engine->gameData.realWater[i];
			if( hI < NO_WATER_INDEX ) {
				wH = engine->gameData.waterHeights[hI];
				// If the vehicle is currently underwater
				if( vH <= wH ) {
					// and was not underwater here in the last tick
					if( _lastHeight >= wH ) {
						// we are for real, underwater
						_inWater = true;
					}
					else if( _inWater == false ) {
						// It's just a tunnel or something, we good.
						_inWater = false;
					}
				}
				else {
					// The water is beneath us
					_inWater = false;
				}
			}
			else {
				_inWater = false;
			}
		}

		if( _inWater ) {
			float oZ = 0.f;
			if( vehicle->type != VehicleData::BOAT ) {
				// dimensions.z doesn't quite fite, so divide by 120 instead of 100.
				oZ = (info->handling.dimensions.z / 2.f) - (info->handling.dimensions.z * (info->handling.percentSubmerged/120.f));

				// Damper motion
				physBody->setDamping(0.95f, 0.9f);
			}
			auto vFwd = getRotation() * glm::vec3(0.f, info->handling.dimensions.y/2.f, 0.f);
			auto vRt = getRotation() * glm::vec3(info->handling.dimensions.x/2.f, 0.f, 0.f);

			applyWaterFloat( vFwd, oZ);
			applyWaterFloat(-vFwd, oZ);
			applyWaterFloat( vRt, oZ);
			applyWaterFloat(-vRt, oZ);
		}
		else {
			physBody->setDamping(0.0f, 0.0f);
		}

		_lastHeight = vH;
	}
}

void VehicleObject::setSteeringAngle(float a)
{
	steerAngle = a;
}

float VehicleObject::getSteeringAngle() const
{
	return steerAngle;
}

void VehicleObject::setThrottle(float t)
{
	throttle = t;
}

float VehicleObject::getThrottle() const
{
	return throttle;
}

void VehicleObject::setBraking(float b)
{
	brake = b;
}

float VehicleObject::getBraking() const
{
	return brake;
}

void VehicleObject::setHandbraking(bool hb)
{
	handbrake = hb;
}

bool VehicleObject::getHandbraking() const
{
	return handbrake;
}

void VehicleObject::ejectAll()
{
	for(std::map<size_t, GameObject*>::iterator it = seatOccupants.begin();
		it != seatOccupants.end();
	) {
		if(it->second->type() == GameObject::Character) {
			CharacterObject* c = static_cast<CharacterObject*>(it->second);
			c->setCurrentVehicle(nullptr, 0);
			c->setPosition(getPosition());
		}
		it = seatOccupants.erase(it);
	}
}

GameObject* VehicleObject::getOccupant(size_t seat)
{
	auto it = seatOccupants.find(seat);
	if( it != seatOccupants.end() ) {
		return it->second;
	}
	return nullptr;
}

void VehicleObject::setOccupant(size_t seat, GameObject* occupant)
{
	auto it = seatOccupants.find(seat);
	if(occupant == nullptr) {
		if(it != seatOccupants.end()) {
			seatOccupants.erase(it);
		}
	}
	else {
		if(it == seatOccupants.end()) {
			seatOccupants.insert({seat, occupant});
		}
	}
}

bool VehicleObject::takeDamage(const GameObject::DamageInfo& dmg)
{
	mHealth -= dmg.hitpoints;
	return true;
}

void VehicleObject::setPartDamaged(unsigned int flag, bool damaged)
{
	if(damaged) {
		damageFlags |= flag;
	}
	else {
		damageFlags = damageFlags & ~flag;
	}
}

unsigned int nameToDamageFlag(const std::string& name)
{
	if(name.find("bonnet") != name.npos) return VehicleObject::DF_Bonnet;
	if(name.find("door_lf") != name.npos) return VehicleObject::DF_Door_lf;
	if(name.find("door_rf") != name.npos) return VehicleObject::DF_Door_rf;
	if(name.find("door_lr") != name.npos) return VehicleObject::DF_Door_lr;
	if(name.find("door_rr") != name.npos) return VehicleObject::DF_Door_rr;
	if(name.find("boot") != name.npos) return VehicleObject::DF_Boot;
	if(name.find("windscreen") != name.npos) return VehicleObject::DF_Windscreen;
	if(name.find("bump_front") != name.npos) return VehicleObject::DF_Bump_front;
	if(name.find("bump_rear") != name.npos) return VehicleObject::DF_Bump_rear;
	if(name.find("wing_lf") != name.npos) return VehicleObject::DF_Wing_lf;
	if(name.find("wing_rf") != name.npos) return VehicleObject::DF_Wing_rf;
	if(name.find("wing_lr") != name.npos) return VehicleObject::DF_Wing_lr;
	if(name.find("wing_rr") != name.npos) return VehicleObject::DF_Wing_rr;
	return 0;
}

bool VehicleObject::isFrameVisible(ModelFrame *frame) const
{
	auto& name = frame->getName();
	bool isDam = name.find("_dam") != name.npos;
	bool isOk = name.find("_ok") != name.npos;
	if(name.find("lo") != name.npos
			|| name.find("_dummy") != name.npos) return false;

	if(isDam || isOk) {
		unsigned int dft = nameToDamageFlag(name);
		if(dft == VehicleObject::DF_Door_lf) return false;
		if(isDam) {
			return (damageFlags & dft) == dft;
		}
		else {
			return (damageFlags & dft) == 0;
		}
	}

	return true;
}

void VehicleObject::applyWaterFloat(const glm::vec3 &relPt, float waterOffset)
{
	auto ws = getPosition() + relPt;
	auto wi = engine->gameData.getWaterIndexAt(ws);
	if(wi != NO_WATER_INDEX) {
		float h = engine->gameData.waterHeights[wi] + waterOffset;

		// Calculate wave height
		h += engine->gameData.getWaveHeightAt(ws);

		if ( ws.z <= h ) {
			float x = (h - ws.z);
			float F = WATER_BUOYANCY_K * x + -WATER_BUOYANCY_C * physBody->getLinearVelocity().z();
			physBody->applyForce(btVector3(0.f, 0.f, F),
								 btVector3(relPt.x, relPt.y, relPt.z));
		}
	}
}

// Dammnit Bullet

class ClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
	btCollisionObject* _self;
public:

	ClosestNotMeRayResultCallback( btCollisionObject* self, const btVector3& from, const btVector3& to )
		: ClosestRayResultCallback( from, to ), _self( self ) {}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
	{
		if( rayResult.m_collisionObject == _self ) {
			return 1.0;
		}
		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};

void *VehicleRaycaster::castRay(const btVector3 &from, const btVector3 &to, btVehicleRaycaster::btVehicleRaycasterResult &result)
{
	ClosestNotMeRayResultCallback rayCallback( _vehicle->physBody, from, to );

	_world->rayTest(from, to, rayCallback);

	if( rayCallback.hasHit() ) {
		const btRigidBody* body = btRigidBody::upcast( rayCallback.m_collisionObject );

		if( body && body->hasContactResponse() ) {
			result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
			result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = rayCallback.m_closestHitFraction;
			return (void*) body;
		}
	}
	return 0;
}
