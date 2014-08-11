#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <sys/stat.h>
#include <data/CollisionModel.hpp>
#include <render/Model.hpp>
#include <engine/Animator.hpp>

VehicleObject::VehicleObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, ModelHandle* model, VehicleDataHandle data, VehicleInfoHandle info, const glm::vec3& prim, const glm::vec3& sec)
	: GameObject(engine, pos, rot, model),
	  steerAngle(0.f), throttle(0.f), brake(0.f), handbrake(false),
	  vehicle(data), info(info), colourPrimary(prim),
	  colourSecondary(sec), collision(nullptr), physBody(nullptr), physVehicle(nullptr)
{
	mHealth = 1000.f;

	collision = new CollisionInstance;
	if( collision->createPhysicsBody(this, data->modelName, nullptr, &info->handling) ) {
		physBody = collision->body;

		physRaycaster = new VehicleRaycaster(this, engine->dynamicsWorld);
		btRaycastVehicle::btVehicleTuning tuning;

		float travel = fabs(info->handling.suspensionUpperLimit - info->handling.suspensionLowerLimit);
		tuning.m_frictionSlip = 2.5f;
		tuning.m_maxSuspensionTravelCm = travel * 100.f;

		physVehicle = new btRaycastVehicle(tuning, physBody, physRaycaster);
		physVehicle->setCoordinateSystem(0, 2, 1);
		//physBody->setActivationState(DISABLE_DEACTIVATION);
		engine->dynamicsWorld->addVehicle(physVehicle);

		float kC = 0.5f;
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

			// Max force slightly more than gravity.
			wi.m_maxSuspensionForce = info->handling.mass * 12.f;
			wi.m_suspensionStiffness = (info->handling.suspensionForce * 10.f);

			//float dampEffect = (info->handling.suspensionDamping) / travel;
			//wi.m_wheelsDampingCompression = wi.m_wheelsDampingRelaxation = dampEffect;

			wi.m_wheelsDampingCompression = kC * 2.f * btSqrt(wi.m_suspensionStiffness);
			wi.m_wheelsDampingRelaxation = kR * 2.f * btSqrt(wi.m_suspensionStiffness);
			wi.m_rollInfluence = 0.45f;
			wi.m_frictionSlip = tuning.m_frictionSlip * (front ? info->handling.tractionBias : 1.f - info->handling.tractionBias);
		}

		// Hide all LOD and damage frames.
		animator = new Animator;
		animator->setModel(model->model);

		for(ModelFrame* f : model->model->frames) {
			auto& name = f->getName();
			bool isDam = name.find("_dam") != name.npos;
			bool isLod = name.find("lo") != name.npos;
			bool isDum = name.find("_dummy") != name.npos;
			/*bool isOk = name.find("_ok") != name.npos;*/
			if(isDam || isLod || isDum ) {
				animator->setFrameVisibility(f, false);
			}

			if( isDum ) {
				_hingedObjects[f] = {
					nullptr,
					nullptr
				};
			}
		}
	}
}

VehicleObject::~VehicleObject()
{
	delete collision;

	engine->dynamicsWorld->removeVehicle(physVehicle);

	delete physVehicle;
	delete physRaycaster;
	delete animator;
	
	ejectAll();
}

void VehicleObject::setPosition(const glm::vec3& pos)
{
	GameObject::setPosition(pos);
	if( physBody ) {
		auto t = physBody->getWorldTransform();
		t.setOrigin(btVector3(pos.x, pos.y, pos.z));
		physBody->setWorldTransform(t);
	}
}

glm::vec3 VehicleObject::getPosition() const
{
	if( physBody ) {
		btVector3 Pos = physBody->getWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z());
	}
	return position;
}

void VehicleObject::setRotation(const glm::quat &orientation)
{
	if( physBody ) {
		auto t = physBody->getWorldTransform();
		t.setRotation(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w));
		physBody->setWorldTransform(t);
	}
	GameObject::setRotation(orientation);
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
	// Moved to tickPhysics
}

void VehicleObject::tickPhysics(float dt)
{
	if(physVehicle) {
		// todo: a real engine function
		float velFac = (info->handling.maxVelocity - physVehicle->getCurrentSpeedKmHour()) / info->handling.maxVelocity;
		float engineForce = info->handling.acceleration * 150.f * throttle * velFac;
		if( fabs(engineForce) >= 0.001f ) physBody->activate(true);

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
				float sign = std::signbit(steerAngle) ? -1.f : 1.f;
				float steer = std::min(info->handling.steeringLock*(3.141f/180.f), std::abs(steerAngle)) * sign;
				auto orient = physBody->getOrientation();

				// Find the local-space velocity
				auto velocity = physBody->getLinearVelocity();
				velocity = velocity.rotate(-orient.getAxis(), orient.getAngle());

				// Rudder force is proportional to velocity.
				float rAngle = steer * (velFac * 0.5f + 0.5f);
				btVector3 rForce = btVector3(1000.f * velocity.y() * rAngle, 0.f, 0.f)
						.rotate(orient.getAxis(), orient.getAngle());
				btVector3 rudderPoint = btVector3(0.f, -info->handling.dimensions.y/2.f, 0.f)
						.rotate(orient.getAxis(), orient.getAngle());
				physBody->applyForce(
							rForce,
							rudderPoint);

				btVector3 rudderVector = btVector3(0.f, 1.f, 0.f)
						.rotate(orient.getAxis(), orient.getAngle());
				physBody->applyForce(
							rudderVector * engineForce * 100.f,
							rudderPoint);


				btVector3 dampforce( 10000.f * velocity.x(), velocity.y() * 100.f, 0.f );
				physBody->applyCentralForce(-dampforce.rotate(orient.getAxis(), orient.getAngle()));
			}
		}

		auto ws = getPosition();
		auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		btVector3 bbmin, bbmax;
		// This is in world space.
		physBody->getAabb(bbmin, bbmax);
		float vH = bbmin.z();
		float wH = 0.f;


		if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
			int i = (wX*WATER_HQ_DATA_SIZE) + wY;
			int hI = engine->gameData.realWater[i];
			if( hI < NO_WATER_INDEX ) {
				wH = engine->gameData.waterHeights[hI];
				wH += engine->gameData.getWaveHeightAt(ws);
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
			float bbZ = info->handling.dimensions.z/2.f;

			float oZ = 0.f;
			oZ = -bbZ/2.f + (bbZ * (info->handling.percentSubmerged/120.f));

			if( vehicle->type != VehicleData::BOAT ) {
				// Damper motion
				physBody->setDamping(0.95f, 0.9f);
			}

			if( vehicle->type == VehicleData::BOAT ) {
				oZ = 0.f;
			}

			// Boats, Buoyancy offset is affected by the orientation of the chassis.
			// Vehicles, it isn't.
			glm::vec3 vFwd = glm::vec3(0.f, info->handling.dimensions.y/2.f, oZ),
					vBack = glm::vec3(0.f, -info->handling.dimensions.y/2.f, oZ);
			glm::vec3 vRt = glm::vec3( info->handling.dimensions.x/2.f, 0.f, oZ),
					vLeft = glm::vec3(-info->handling.dimensions.x/2.f, 0.f, oZ);

			vFwd = getRotation() * vFwd;
			vBack = getRotation() * vBack;
			vRt = getRotation() * vRt;
			vLeft = getRotation() * vLeft;

			// This function will try to keep v* at the water level.
			applyWaterFloat( vFwd);
			applyWaterFloat( vBack);
			applyWaterFloat( vRt);
			applyWaterFloat( vLeft);
		}
		else {
			if( vehicle->type == VehicleData::BOAT ) {
				physBody->setDamping(0.1f, 0.8f);
			}
			else {
				physBody->setDamping(0.05f, 0.0f);
			}
		}

		_lastHeight = vH;

		// Update hinge object rotations
		for(auto it : _hingedObjects) {
			if(it.second.body == nullptr) continue;
			auto inv = glm::inverse(getRotation());
			auto rot = it.second.body->getWorldTransform().getRotation();
			animator->setFrameOrientation(it.first,
					inv * glm::quat(rot.w(), rot.x(), rot.y(), rot.z()));
		}
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

	const float frameDamageThreshold = 1500.f;

	if( dmg.impulse >= frameDamageThreshold ) {
		auto dpoint = dmg.damageLocation;
		dpoint -= getPosition();
		dpoint = glm::inverse(getRotation()) * dpoint;

		// find visible "_ok" frames and damage them.
		for(ModelFrame* f : model->model->frames) {
			auto& name = f->getName();
			if( name.find("_ok") != name.npos ) {
				auto& geom = model->model->geometries[f->getGeometries()[0]];
				auto pp = f->getMatrix() * glm::vec4(0.f, 0.f, 0.f, 1.f);
				float td = glm::distance(glm::vec3(pp)+geom->geometryBounds.center
										 , dpoint);
				if( td < geom->geometryBounds.radius * 1.2f ) {
					setFrameState(f, DAM);

					// Disable the lock on damaged frames.
					setHingeLocked(f->getParent(), false);
				}
			}
		}
	}

	return true;
}

void VehicleObject::setFrameState(ModelFrame* f, FrameState state)
{
	bool isOkVis = animator->getFrameVisibility(f);

	auto damName = f->getName();
	damName.replace(damName.find("_ok"), 3, "_dam");
	auto damage = model->model->findFrame(damName);

	if(DAM == state) {
		if( isOkVis ) {
			animator->setFrameVisibility(f, false);
			animator->setFrameVisibility(damage, true);
		}
	}
	else if(OK == state) {
		if(! isOkVis) {
			animator->setFrameVisibility(f, true);
			animator->setFrameVisibility(damage, false);
		}
	}
}

void VehicleObject::applyWaterFloat(const glm::vec3 &relPt)
{
	auto ws = getPosition() + relPt;
	auto wi = engine->gameData.getWaterIndexAt(ws);
	if(wi != NO_WATER_INDEX) {
		float h = engine->gameData.waterHeights[wi];

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

void VehicleObject::setHingeLocked(ModelFrame *frame, bool locked)
{
	auto hit = _hingedObjects.find(frame);
	if( hit != _hingedObjects.end() ) {
		if( !locked && hit->second.body == nullptr ) {
			createObjectHinge(physBody->getWorldTransform(), frame);
		}
		else if( locked && hit->second.body ) {
			destroyObjectHinge(hit->second);
		}
	}
}

void VehicleObject::createObjectHinge(btTransform& local, ModelFrame *frame)
{
	float sign = glm::sign(frame->getDefaultTranslation().x);
	btVector3 hingeAxis,
			hingePosition;
	btVector3 boxSize,
			boxOffset;
	float hingeMax = 1.f;
	float hingeMin = 0.f;

	auto& fn = frame->getName();

	if( frame->getChildren().size() == 0 ) return;

	ModelFrame* okframe = frame->getChildren()[0];

	if( okframe->getGeometries().size() == 0 ) return;
	auto& geom = model->model->geometries[okframe->getGeometries()[0]];
	auto gbounds = geom->geometryBounds;

	if( fn.find("door") != fn.npos ) {
		std::cout << fn << std::endl;
		hingeAxis = {0.f, 0.f, 1.f};
		hingePosition = {0.f, 0.2f, 0.f};
		boxSize = {0.1f, 0.4f, gbounds.radius/2.f};
		boxOffset = {0.f,-0.2f, gbounds.center.z/2.f};
		if( sign > 0.f ) {
			hingeMin = -glm::quarter_pi<float>() * 1.5f;
			hingeMax = 0.f;
		}
		else {
			hingeMax = glm::quarter_pi<float>() * 1.5f;
			hingeMin = 0.f;
		}
	}
	else if( fn.find("bonnet") != fn.npos ) {
		hingeAxis = {1.f, 0.f, 0.f};
		hingePosition = {0.f, -0.2f, 0.f};
		hingeMax = 0.f;
		hingeMin = -glm::quarter_pi<float>() * 1.5f;
		boxSize = {0.4f, 0.4f, 0.1f};
		boxOffset = {0.f, 0.2f, 0.f};
	}
	else {
		// TODO: boot, bumper
		return;
	}

	btDefaultMotionState* dms = new btDefaultMotionState();
	btTransform tr = btTransform::getIdentity();

	auto p = frame->getDefaultTranslation();
	auto o = glm::toQuat(frame->getDefaultRotation());
	auto bp = btVector3(p.x, p.y, p.z);
	tr.setOrigin(bp);
	tr.setRotation(btQuaternion(o.x, o.y, o.z, o.w));

	dms->setWorldTransform(local * tr);

	btCompoundShape* cs = new btCompoundShape;
	btCollisionShape* bshape = new btBoxShape( boxSize );
	btTransform t; t.setIdentity();
	t.setOrigin(boxOffset);
	cs->addChildShape(t, bshape);

	btVector3 inertia;
	cs->calculateLocalInertia(10.f, inertia);

	btRigidBody::btRigidBodyConstructionInfo rginfo(10.f, dms, cs, inertia);
	btRigidBody* subObject = new btRigidBody(rginfo);
	subObject->setUserPointer(this);

	auto hinge = new btHingeConstraint(
				*physBody,
				*subObject,
				bp, hingePosition,
				hingeAxis, hingeAxis);
	hinge->setLimit(hingeMin, hingeMax);

	engine->dynamicsWorld->addRigidBody(subObject);
	engine->dynamicsWorld->addConstraint(hinge, true);

	_hingedObjects[frame].body = subObject;
	_hingedObjects[frame].constraint = hinge;
}

void VehicleObject::destroyObjectHinge(VehicleObject::HingeInfo &hinge)
{
	if( hinge.body ) {
		engine->dynamicsWorld->removeRigidBody(hinge.body);
		engine->dynamicsWorld->removeConstraint(hinge.constraint);

		delete hinge.body;
		delete hinge.constraint;
		hinge.body = nullptr;
		hinge.constraint = nullptr;
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

	const void *res = 0;

	_world->rayTest(from, to, rayCallback);

	if( rayCallback.hasHit() ) {
		const btRigidBody* body = btRigidBody::upcast( rayCallback.m_collisionObject );

		if( body && body->hasContactResponse() ) {
			result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
			result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = rayCallback.m_closestHitFraction;
			res = body;
		}
	}

	return (void* )res;
}
