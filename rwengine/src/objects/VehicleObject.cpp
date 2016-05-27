#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>
#include <dynamics/CollisionInstance.hpp>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <dynamics/RaycastCallbacks.hpp>
#include <data/CollisionModel.hpp>
#include <data/Skeleton.hpp>
#include <data/Model.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>

#define PART_CLOSE_VELOCITY 0.25f
constexpr float kVehicleMaxExitVelocity = 0.15f;

/**
 * A raycaster that will ignore the body of the vehicle when casting rays
 */
class VehicleRaycaster : public btVehicleRaycaster
{
	btDynamicsWorld* _world;
	VehicleObject* _vehicle;
public:
	VehicleRaycaster(VehicleObject* vehicle, btDynamicsWorld* world)
		: _world(world), _vehicle(vehicle) {}

	void* castRay(const btVector3 &from, const btVector3 &to, btVehicleRaycasterResult &result)
	{
		ClosestNotMeRayResultCallback rayCallback( _vehicle->collision->getBulletBody(), from, to );
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
};

VehicleObject::VehicleObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model, VehicleDataHandle data, VehicleInfoHandle info, const glm::u8vec3& prim, const glm::u8vec3& sec)
	: GameObject(engine, pos, rot, model)
	, steerAngle(0.f)
	, throttle(0.f)
	, brake(0.f)
	, handbrake(true)
	, vehicle(data)
	, info(info)
	, colourPrimary(prim)
	, colourSecondary(sec)
	, collision(nullptr)
	, physRaycaster(nullptr)
	, physVehicle(nullptr)
{
	collision = new CollisionInstance;
	if( collision->createPhysicsBody(this, data->modelName, nullptr, &info->handling) ) {

		physRaycaster = new VehicleRaycaster(this, engine->dynamicsWorld);
		btRaycastVehicle::btVehicleTuning tuning;

		float travel = fabs(info->handling.suspensionUpperLimit - info->handling.suspensionLowerLimit);
		tuning.m_frictionSlip = 3.f;
		tuning.m_maxSuspensionTravelCm = travel * 100.f;

		physVehicle = new btRaycastVehicle(tuning, collision->getBulletBody(), physRaycaster);
		physVehicle->setCoordinateSystem(0, 2, 1);
		//physBody->setActivationState(DISABLE_DEACTIVATION);
		engine->dynamicsWorld->addAction(physVehicle);

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
			wi.m_raycastInfo.m_suspensionLength = 0.f;

			wi.m_maxSuspensionForce = info->handling.mass * 9.f;
			wi.m_suspensionStiffness = (info->handling.suspensionForce * 50.f);

			//float dampEffect = (info->handling.suspensionDamping) / travel;
			//wi.m_wheelsDampingCompression = wi.m_wheelsDampingRelaxation = dampEffect;

			wi.m_wheelsDampingCompression = kC * 2.f * btSqrt(wi.m_suspensionStiffness);
			wi.m_wheelsDampingRelaxation = kR * 2.f * btSqrt(wi.m_suspensionStiffness);
			wi.m_rollInfluence = 0.30f;
			float halfFriction = tuning.m_frictionSlip * 0.5f;
			wi.m_frictionSlip = halfFriction + halfFriction * (front ? info->handling.tractionBias : 1.f - info->handling.tractionBias);
		}

		// Hide all LOD and damage frames.
		skeleton = new Skeleton;
		
		for(ModelFrame* frame : model->resource->frames)
		{
			auto& name = frame->getName();
			bool isDam = name.find("_dam") != name.npos;
			bool isLod = name.find("lo") != name.npos;
			bool isDum = name.find("_dummy") != name.npos;
			/*bool isOk = name.find("_ok") != name.npos;*/
			if(isDam || isLod || isDum ) {
				skeleton->setEnabled(frame, false);
			}

			if( isDum ) {
				registerPart(frame);
			}
		}
	}
}

VehicleObject::~VehicleObject()
{
	ejectAll();
	
	engine->dynamicsWorld->removeAction(physVehicle);
	
	for(auto& p : dynamicParts)
	{
		setPartLocked(&p.second, true);
	}
	
	delete collision;

	delete physVehicle;
	delete physRaycaster;
}

void VehicleObject::setPosition(const glm::vec3& pos)
{
	GameObject::setPosition(pos);
	if( collision->getBulletBody() ) {
		auto bodyOrigin = btVector3(position.x, position.y, position.z);
		for(auto& part : dynamicParts)
		{
			if( part.second.body == nullptr ) continue;
			auto body = part.second.body;
			auto rel = body->getWorldTransform().getOrigin() -
					bodyOrigin;
			body->getWorldTransform().setOrigin(
				btVector3(pos.x + rel.x(), pos.y + rel.y(), pos.z + rel.z()));
		}

		auto t = collision->getBulletBody()->getWorldTransform();
		t.setOrigin(btVector3(pos.x, pos.y, pos.z));
		collision->getBulletBody()->setWorldTransform(t);
	}
}

void VehicleObject::setRotation(const glm::quat &orientation)
{
	if( collision->getBulletBody() ) {
		auto t = collision->getBulletBody()->getWorldTransform();
		t.setRotation(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w));
		collision->getBulletBody()->setWorldTransform(t);
	}
	GameObject::setRotation(orientation);
}

#include <glm/gtc/type_ptr.hpp>

void VehicleObject::tick(float dt)
{
	RW_UNUSED(dt);
	// Moved to tickPhysics
}

void VehicleObject::tickPhysics(float dt)
{
	RW_UNUSED(dt);

	if( physVehicle )
	{
		// todo: a real engine function
		float velFac = info->handling.maxVelocity;
		float engineForce = info->handling.acceleration * throttle * velFac;
		if( fabs(engineForce) >= 0.001f )
		{
			collision->getBulletBody()->activate(true);
		}
		
		float brakeF = getBraking();
		
		if( handbrake )
		{
			brakeF = 5.f;
		}

		for(int w = 0; w < physVehicle->getNumWheels(); ++w) {
			btWheelInfo& wi = physVehicle->getWheelInfo(w);
			if( info->handling.driveType == VehicleHandlingInfo::All ||
					(info->handling.driveType == VehicleHandlingInfo::Forward && wi.m_bIsFrontWheel) ||
					(info->handling.driveType == VehicleHandlingInfo::Rear && !wi.m_bIsFrontWheel))
			{
					physVehicle->applyEngineForce(engineForce, w);
			}

			float brakeReal = 5.f * info->handling.brakeDeceleration * (wi.m_bIsFrontWheel? info->handling.brakeBias : 1.f - info->handling.brakeBias);
			physVehicle->setBrake(brakeReal * brakeF, w);

			if(wi.m_bIsFrontWheel) {
				float sign = std::signbit(steerAngle) ? -1.f : 1.f;
				physVehicle->setSteeringValue(std::min(info->handling.steeringLock*(3.141f/180.f), std::abs(steerAngle)) * sign, w);
				//physVehicle->setSteeringValue(std::min(3.141f/2.f, std::abs(steerAngle)) * sign, w);
			}
		}

		// Update passenger positions
		for (auto& seat : seatOccupants)
		{
			auto character = static_cast<CharacterObject*>(seat.second);

			glm::vec3 passPosition;
			if (character->isEnteringOrExitingVehicle())
			{
				passPosition = getSeatEntryPositionWorld(seat.first);
			}
			else
			{
				passPosition = getPosition();
				if (seat.first < info->seats.size()) {
					passPosition += getRotation() * (info->seats[seat.first].offset);
				}
			}
			seat.second->updateTransform(passPosition, getRotation());
		}

		if( vehicle->type == VehicleData::BOAT ) {
			if( isInWater() ) {
				float sign = std::signbit(steerAngle) ? -1.f : 1.f;
				float steer = std::min(info->handling.steeringLock*(3.141f/180.f), std::abs(steerAngle)) * sign;
				auto orient = collision->getBulletBody()->getOrientation();

				// Find the local-space velocity
				auto velocity = collision->getBulletBody()->getLinearVelocity();
				velocity = velocity.rotate(-orient.getAxis(), orient.getAngle());

				// Rudder force is proportional to velocity.
				float rAngle = steer * (velFac * 0.5f + 0.5f);
				btVector3 rForce = btVector3(1000.f * velocity.y() * rAngle, 0.f, 0.f)
						.rotate(orient.getAxis(), orient.getAngle());
				btVector3 rudderPoint = btVector3(0.f, -info->handling.dimensions.y/2.f, 0.f)
						.rotate(orient.getAxis(), orient.getAngle());
				collision->getBulletBody()->applyForce(
							rForce,
							rudderPoint);

				btVector3 rudderVector = btVector3(0.f, 1.f, 0.f)
						.rotate(orient.getAxis(), orient.getAngle());
				collision->getBulletBody()->applyForce(
							rudderVector * engineForce * 100.f,
							rudderPoint);


				btVector3 dampforce( 10000.f * velocity.x(), velocity.y() * 100.f, 0.f );
				collision->getBulletBody()->applyCentralForce(-dampforce.rotate(orient.getAxis(), orient.getAngle()));
			}
		}

		const auto& ws = getPosition();
		auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
		btVector3 bbmin, bbmax;
		// This is in world space.
		collision->getBulletBody()->getAabb(bbmin, bbmax);
		float vH = bbmin.z();
		float wH = 0.f;


		if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
			int i = (wX*WATER_HQ_DATA_SIZE) + wY;
			int hI = engine->data->realWater[i];
			if( hI < NO_WATER_INDEX ) {
				wH = engine->data->waterHeights[hI];
				wH += engine->data->getWaveHeightAt(ws);
				// If the vehicle is currently underwater
				if( vH <= wH ) {
					// and was not underwater here in the last tick
					if( _lastHeight >= wH ) {
						// we are for real, underwater
						inWater = true;
					}
					else if( inWater == false ) {
						// It's just a tunnel or something, we good.
						inWater = false;
					}
				}
				else {
					// The water is beneath us
					inWater = false;
				}
			}
			else {
				inWater = false;
			}
		}

		if( inWater ) {
			// Ensure that vehicles don't fall asleep at the top of a wave.
			if(! collision->getBulletBody()->isActive() )
			{
				collision->getBulletBody()->activate(true);
			}
			
			float bbZ = info->handling.dimensions.z/2.f;

			float oZ = 0.f;
			oZ = -bbZ/2.f + (bbZ * (info->handling.percentSubmerged/120.f));

			if( vehicle->type != VehicleData::BOAT ) {
				// Damper motion
				collision->getBulletBody()->setDamping(0.95f, 0.9f);
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
				collision->getBulletBody()->setDamping(0.1f, 0.8f);
			}
			else {
				collision->getBulletBody()->setDamping(0.05f, 0.0f);
			}
		}

		_lastHeight = vH;

		// Update hinge object rotations
		for(auto& it : dynamicParts) {
			if(it.second.body == nullptr) continue;
			auto inv = glm::inverse(getRotation());
			auto rot = it.second.body->getWorldTransform().getRotation();
			//auto pos = it.second.body->getWorldTransform().getOrigin();
			auto r2 = inv * glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
			//auto p2 = inv * (glm::vec3(pos.x(), pos.y(), pos.z()) - getPosition());
			
			auto& prev = skeleton->getData(it.second.dummy->getIndex()).a;
			auto next = prev;
			next.rotation = r2;
			//next.translation = p2;
			skeleton->setData(it.second.dummy->getIndex(), { next, prev, true } );
			
			if( it.second.moveToAngle )
			{
				auto angledelta = it.second.targetAngle - it.second.constraint->getHingeAngle();
				if( glm::abs(angledelta) <= 0.01f )
				{
					it.second.constraint->enableAngularMotor(false, 1.f, 1.f);
					dynamicParts[it.first].moveToAngle = false;
				}
				else
				{
					it.second.constraint->enableAngularMotor(true, glm::sign(angledelta) * 5.f, 1.f);
				}
			}
			
			// If the part is moving quite fast and near the limit, lock it.
			/// @TODO not all parts rotate in the z axis.
			float zspeed = it.second.body->getAngularVelocity().getZ();
			if(it.second.openAngle < 0.f) zspeed = -zspeed;
			if(zspeed >= PART_CLOSE_VELOCITY)
			{
				auto d = it.second.constraint->getHingeAngle() - it.second.closedAngle;
				if( glm::abs(d) < 0.05f )
				{
					dynamicParts[it.first].moveToAngle = false;
					setPartLocked(&(it.second), true);
				}
			}
		}
	}
}

bool VehicleObject::isFlipped() const
{
	auto up = getRotation() * glm::vec3(0.f, 0.f, 1.f);
	return up.z <= -0.1f;
}

float VehicleObject::getVelocity() const
{
	if (physVehicle) {
		return (physVehicle->getCurrentSpeedKmHour()*1000.f)/(60.f*60.f);
	}
	return 0.f;
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
		seatOccupants[seat] = occupant;
	}
}

bool VehicleObject::canOccupantExit() const
{
	return getVelocity() <= kVehicleMaxExitVelocity;
}

bool VehicleObject::isOccupantDriver(size_t seat) const
{
	// This isn't true for all vehicles, but it'll do until we figure it out
	return seat == 0;
}

VehicleObject::Part* VehicleObject::getSeatEntryDoor(size_t seat)
{
	auto pos = info->seats.at(seat).offset + glm::vec3(0.f, 0.5f, 0.f);
	Part* nearestDoor = nullptr;
	float d = std::numeric_limits<float>::max();
	for(auto& p : dynamicParts)
	{
		float partDist = glm::distance(p.second.dummy->getDefaultTranslation(), pos);
		if( partDist < d && p.second.dummy->getName().substr(0, 5) == "door_" )
		{
			d = partDist;
			nearestDoor = &p.second;
		}
	}
	return nearestDoor;
}

bool VehicleObject::takeDamage(const GameObject::DamageInfo& dmg)
{
	RW_CHECK(dmg.hitpoints == 0, "Vehicle Damage not implemented yet");

	const float frameDamageThreshold = 1500.f;

	if( dmg.impulse >= frameDamageThreshold ) {
		auto dpoint = dmg.damageLocation;
		dpoint -= getPosition();
		dpoint = glm::inverse(getRotation()) * dpoint;

		// Set any parts within range to damaged state.
		for(auto d : dynamicParts)
		{
			auto p = &d.second;
			
			if( p->normal == nullptr ) continue;
			
			if( skeleton->getData(p->normal->getIndex()).enabled )
			{
				auto& geom = model->resource->geometries[p->normal->getGeometries()[0]];
				auto pp = p->normal->getMatrix() * glm::vec4(0.f, 0.f, 0.f, 1.f);
				float td = glm::distance(glm::vec3(pp)+geom->geometryBounds.center
										 , dpoint);
				if( td < geom->geometryBounds.radius * 1.2f ) {
					setPartState(p, DAM);
					setPartLocked(p, false);
				}
			}
		}
	}

	return true;
}

void VehicleObject::setPartState(VehicleObject::Part* part, VehicleObject::FrameState state)
{
	if( state == VehicleObject::OK )
	{
		if( part->normal ) skeleton->setEnabled(part->normal, true);
		if( part->damaged ) skeleton->setEnabled(part->damaged, false);
	}
	else if( state == VehicleObject::DAM )
	{
		if( part->normal ) skeleton->setEnabled(part->normal, false);
		if( part->damaged ) skeleton->setEnabled(part->damaged, true);
	}
}

void VehicleObject::applyWaterFloat(const glm::vec3 &relPt)
{
	auto ws = getPosition() + relPt;
	auto wi = engine->data->getWaterIndexAt(ws);
	if(wi != NO_WATER_INDEX) {
		float h = engine->data->waterHeights[wi];

		// Calculate wave height
		h += engine->data->getWaveHeightAt(ws);

		if ( ws.z <= h ) {
			float x = (h - ws.z);
			float F = WATER_BUOYANCY_K * x +
					-WATER_BUOYANCY_C * collision->getBulletBody()->getLinearVelocity().z();
			collision->getBulletBody()->applyImpulse(btVector3(0.f, 0.f, F),
								 btVector3(relPt.x, relPt.y, relPt.z));
		}
	}
}

void VehicleObject::setPartLocked(VehicleObject::Part* part, bool locked)
{
	if( part->body == nullptr && locked == false )
	{
		createObjectHinge(collision->getBulletBody()->getWorldTransform(), part);
	}
	else if( part->body != nullptr && locked == true )
	{
		destroyObjectHinge(part);
		
		// Restore default bone transform
		auto dt = part->dummy->getDefaultTranslation();
		auto dr = glm::quat_cast(part->dummy->getDefaultRotation());
		Skeleton::FrameTransform tf { dt, dr };
		skeleton->setData(part->dummy->getIndex(), { tf, tf, true });
	}
}

void VehicleObject::setPartTarget(VehicleObject::Part* part, bool enable, float target)
{
	if( enable )
	{
		if( part->body == nullptr )
		{
			setPartLocked(part, false);
		}
		
		part->targetAngle = target;
		part->moveToAngle = true;

		part->body->activate(true);
	}
	else
	{
		part->targetAngle = target;
		part->moveToAngle = false;
	}
}

VehicleObject::Part* VehicleObject::getPart(const std::string& name)
{
	auto f = dynamicParts.find(name);
	if( f != dynamicParts.end() )
	{
		return &f->second;
	}
	return nullptr;
}

ModelFrame* findStateFrame(ModelFrame* f, const std::string& state)
{
	auto it = std::find_if(
		f->getChildren().begin(),
		f->getChildren().end(),
			[&](ModelFrame* c){ return c->getName().find(state) != std::string::npos; }
							);
	if( it != f->getChildren().end() )
	{
		return *it;
	}
	return nullptr;
}

void VehicleObject::registerPart(ModelFrame* mf)
{
	auto normal = findStateFrame(mf, "_ok");
	auto damage = findStateFrame(mf, "_dam");
	
	if( normal == nullptr && damage == nullptr )
	{
		// Not actually a useful part, just a dummy.
		return;
	}
	
	dynamicParts.insert(
		{ mf->getName(), 
			{
				mf,
				normal,
				damage,
				nullptr, nullptr,
				false,
				0.f,
				0.f,
				0.f
			}
		});
}

void VehicleObject::createObjectHinge(btTransform& local, Part *part)
{
	float sign = glm::sign(part->dummy->getDefaultTranslation().x);
	btVector3 hingeAxis,
			hingePosition;
	btVector3 boxSize,
			boxOffset;
	float hingeMax = 1.f;
	float hingeMin = 0.f;

	auto& fn = part->dummy->getName();

	ModelFrame* okframe = part->normal;

	if( okframe->getGeometries().size() == 0 ) return;
	
	auto& geom = model->resource->geometries[okframe->getGeometries()[0]];
	auto gbounds = geom->geometryBounds;
	
	if( fn.find("door") != fn.npos ) {
		hingeAxis = {0.f, 0.f, 1.f};
		//hingePosition = {0.f, 0.2f, 0.f};
		boxSize = {0.15f, 0.5f, 0.6f};
		//boxOffset = {0.f,-0.2f, gbounds.center.z/2.f};
		auto tf = gbounds.center;
		boxOffset = btVector3(tf.x, tf.y, tf.z);
		hingePosition = -boxOffset;
		
		if( sign < 0.f ) {
			hingeMax = glm::quarter_pi<float>() * 1.5f;
			hingeMin = 0.f;
			part->openAngle = hingeMax;
			part->closedAngle = hingeMin;
		}
		else
		{
			hingeMin = glm::quarter_pi<float>() * -1.5f;
			hingeMax = 0.f;
			part->openAngle = hingeMin;
			part->closedAngle = hingeMax;
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

	auto p = part->dummy->getDefaultTranslation();
	auto o = glm::toQuat(part->dummy->getDefaultRotation());
	tr.setOrigin(btVector3(p.x, p.y, p.z));
	tr.setRotation(btQuaternion(o.x, o.y, o.z, o.w));
	dms->setWorldTransform(local * tr);

	btCollisionShape* cs = new btBoxShape( boxSize );
	btTransform t; t.setIdentity();
	t.setOrigin(boxOffset);

	btVector3 inertia;
	cs->calculateLocalInertia(10.f, inertia);

	btRigidBody::btRigidBodyConstructionInfo rginfo(10.f, dms, cs, inertia);
	btRigidBody* subObject = new btRigidBody(rginfo);
	subObject->setUserPointer(this);

	auto hinge = new btHingeConstraint(
				*collision->getBulletBody(),
				*subObject,
				tr.getOrigin(), hingePosition,
				hingeAxis, hingeAxis);
	hinge->setLimit(hingeMin, hingeMax);
	hinge->setBreakingImpulseThreshold(250.f);
	
	engine->dynamicsWorld->addRigidBody(subObject);
	engine->dynamicsWorld->addConstraint(hinge, true);
	
	part->body = subObject;
	part->constraint = hinge;
}

void VehicleObject::destroyObjectHinge(Part* part)
{
	if( part->body != nullptr ) {
		engine->dynamicsWorld->removeConstraint(part->constraint);
		engine->dynamicsWorld->removeRigidBody(part->body);

		delete part->body;
		delete part->constraint;
		
		part->body = nullptr;
		part->constraint = nullptr;
		
		// Reset target.
		part->moveToAngle = false;
	}
}

void VehicleObject::setPrimaryColour(uint8_t color)
{
	colourPrimary = engine->data->vehicleColours[color];
}

void VehicleObject::setSecondaryColour(uint8_t color)
{
	colourSecondary = engine->data->vehicleColours[color];
}

